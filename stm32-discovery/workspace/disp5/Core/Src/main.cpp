/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "crc.h"
#include "dma.h"
#include "dma2d.h"
#include "ltdc.h"
#include "quadspi.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "lidar.h"

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POINTS 1800
#define PACKET_SIZE 5
#define BLOCK_SIZE 132
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile bool received_flag = false;
volatile bool parseData = false;
volatile bool lockArrays = false;
uint8_t packet[PACKET_SIZE] = {0};
uint8_t buff[3*BLOCK_SIZE] = {0};

uint32_t cnt = 0;


float amax = 0;
float dmax = 0;
float amin = 0;
float dmin = 1000.0;

float angles[POINTS] = {0.0};
float distances[POINTS] = {0.0};

uint8_t fakerecv = 0;

//#### LIDAR COMMANDS ####
uint8_t reset[] = {0xA5, 0x40};
uint8_t startscan[] = {0xA5, 0x20};
uint8_t startscan_extended[] = {0xA5, 0x82, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x5d};

//UNUSED BUT USEFUL!
uint8_t get_salmplerate[] = {0xA5, 0x59};
uint8_t get_health[] = {0xA5, 0x52};
uint8_t get_info[] = {0xA5, 0x50};
uint8_t get_lidar_conf[] = {0xA5, 0x84};
uint8_t stop_rq[] = {0xA5, 0x25};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */


#define ULTRA_CABINS_IN_RESPONSE 32
#define CABIN_SIZE 4
#define ANGLE_OFFSET_14_8 3
#define ANGLE_OFFSET_07_0 2
#define ULTRA_CABIN_0_OFFSET 4
#define RPLIDAR_VARBITSCALE_X2_SRC_BIT  9
#define RPLIDAR_VARBITSCALE_X4_SRC_BIT  11
#define RPLIDAR_VARBITSCALE_X8_SRC_BIT  12
#define RPLIDAR_VARBITSCALE_X16_SRC_BIT 14

#define RPLIDAR_VARBITSCALE_X2_DEST_VAL 512
#define RPLIDAR_VARBITSCALE_X4_DEST_VAL 1280
#define RPLIDAR_VARBITSCALE_X8_DEST_VAL 1792
#define RPLIDAR_VARBITSCALE_X16_DEST_VAL 3328
#define RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT  2


#define RPLIDAR_VARBITSCALE_GET_SRC_MAX_VAL_BY_BITS(_BITS_) \
    (  (((0x1<<(_BITS_)) - RPLIDAR_VARBITSCALE_X16_DEST_VAL)<<4) + \
       ((RPLIDAR_VARBITSCALE_X16_DEST_VAL - RPLIDAR_VARBITSCALE_X8_DEST_VAL)<<3) + \
       ((RPLIDAR_VARBITSCALE_X8_DEST_VAL - RPLIDAR_VARBITSCALE_X4_DEST_VAL)<<2) + \
       ((RPLIDAR_VARBITSCALE_X4_DEST_VAL - RPLIDAR_VARBITSCALE_X2_DEST_VAL)<<1) + \
       RPLIDAR_VARBITSCALE_X2_DEST_VAL - 1)

static uint32_t _varbitscale_decode(uint32_t scaled, uint32_t & scaleLevel)
{
    static const uint32_t VBS_SCALED_BASE[] = {
        RPLIDAR_VARBITSCALE_X16_DEST_VAL,
        RPLIDAR_VARBITSCALE_X8_DEST_VAL,
        RPLIDAR_VARBITSCALE_X4_DEST_VAL,
        RPLIDAR_VARBITSCALE_X2_DEST_VAL,
        0,
    };

    static const uint32_t VBS_SCALED_LVL[] = {
        4,
        3,
        2,
        1,
        0,
    };

    static const uint32_t VBS_TARGET_BASE[] = {
        (0x1 << RPLIDAR_VARBITSCALE_X16_SRC_BIT),
        (0x1 << RPLIDAR_VARBITSCALE_X8_SRC_BIT),
        (0x1 << RPLIDAR_VARBITSCALE_X4_SRC_BIT),
        (0x1 << RPLIDAR_VARBITSCALE_X2_SRC_BIT),
        0,
    };

    for (size_t i = 0; i < 5; ++i)
    {
        int remain = ((int)scaled - (int)VBS_SCALED_BASE[i]);
        if (remain >= 0) {
            scaleLevel = VBS_SCALED_LVL[i];
            return VBS_TARGET_BASE[i] + (remain << scaleLevel);
        }
    }
    return 0;
}


//DMA data-packet interrupt
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	static uint8_t k = 0;
	static uint8_t last_k = 2;
	static uint8_t next_k = 1;



	if(parseData)
	{


		HAL_UART_Receive_DMA(&huart6, buff + next_k * BLOCK_SIZE, BLOCK_SIZE);

		int current_start_angle_q8 = (((uint16_t)(*(buff + k * BLOCK_SIZE + ANGLE_OFFSET_14_8)) & 0x7f) << 10);
		current_start_angle_q8 |= (*(buff + k * BLOCK_SIZE + ANGLE_OFFSET_07_0)) << 2;

		int previous_start_angle_q8 = (((uint16_t)(*(buff + last_k * BLOCK_SIZE + ANGLE_OFFSET_14_8)) & 0x7f) << 10);
		previous_start_angle_q8 |= (*(buff + last_k * BLOCK_SIZE + ANGLE_OFFSET_07_0)) << 2;


		int start_angle_diff_q8 = current_start_angle_q8 - previous_start_angle_q8;
		if(previous_start_angle_q8 > current_start_angle_q8) start_angle_diff_q8 += (360 << 8);
		int angle_step_q16 = (start_angle_diff_q8 << 3) / 3;
		int current_raw_angle_q16 = (previous_start_angle_q8 << 8);



		if(previous_start_angle_q8 > 0)
		{
			for(uint8_t i = 0; i < ULTRA_CABINS_IN_RESPONSE ; i++)
			{
				int dist_q2[3];
				int angle_q6[3];
				int syncBit[3];

				uint32_t combined = (*((uint32_t*)(buff + last_k * BLOCK_SIZE + ULTRA_CABIN_0_OFFSET + CABIN_SIZE * i)));

				//'signed, do not touch'
				int dist_major = (combined & 0xfff);

				int dist_predict1 = (((int)(combined << 10)) >> 22);
				int dist_predict2 = (((int)combined) >> 22);

				int dist_major2;
				uint32_t scale1;
				uint32_t scale2;


				//prefetch
				if(i == ULTRA_CABINS_IN_RESPONSE - 1){
					dist_major2 = (*((uint32_t*)(buff + k * BLOCK_SIZE + ULTRA_CABIN_0_OFFSET + CABIN_SIZE * 0))) & 0xfff;
				}else{
					dist_major2 = (*((uint32_t*)(buff + last_k * BLOCK_SIZE + ULTRA_CABIN_0_OFFSET + CABIN_SIZE * (i + 1)))) & 0xfff;
				}

				dist_major = _varbitscale_decode(dist_major, scale1);
				dist_major2 = _varbitscale_decode(dist_major2, scale2);

				int dist_base1 = dist_major;
				int dist_base2 = dist_major2;

				if((!dist_major) && dist_major2){
					dist_base1 = dist_major2;
					scale1 = scale2;
				}

				dist_q2[0] = (dist_major << 2);
				if((dist_predict1 == 0xFFFFFE00) || (dist_predict1 == 0x1FF)){
					dist_q2[1] = 0;
				} else {
					dist_predict1 = (dist_predict1 << scale1);
					dist_q2[1] = (dist_predict1 + dist_base1) << 2;
				}

				if((dist_predict1 == 0xFFFFFE00) || (dist_predict2 == 0x1FF)){
					dist_q2[2] = 0;
				} else {
					dist_predict2 =  (dist_predict2 << scale2);
					dist_q2[2] = (dist_predict2 + dist_base2) << 2;
				}


				for(int j = 0; j < 3; j++){
					syncBit[j] = (((current_raw_angle_q16 + angle_step_q16) % (360 << 16)) < angle_step_q16) ? 1 : 0;
					int offset_angle_mean_q16 = (int)(7.5 * 3.1415926535 * (1<<16) / 180.0);

					if(dist_q2[j] >= (50*4)){
						const int k1 = 98361;
						const int k2 = (int)(k1/dist_q2[j]);
						offset_angle_mean_q16 = (int)((8 * 3.1415926535 * (1<<16) / 180) - (k2 << 6) - (k2 * k2 * k2) / 98304);
					}

					angle_q6[j] = ((current_raw_angle_q16 - int(offset_angle_mean_q16 * 180 / 3.14159265)) >> 10);
					current_raw_angle_q16 += angle_step_q16;

					if(angle_q6[j] < 0) angle_q6[j] += (360 << 6);
					else if(angle_q6[j] >= (360 << 6)) angle_q6[j] -= (360 << 6);

					float f_angle = ((float) angle_q6[j])/ 64.0f;
					float f_distance = ((float) dist_q2[j]) / 4.0f;

					uint16_t quality = dist_q2[j] ? (0x2f << RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT) : 0;

					if(f_angle >1.0 && f_angle < 360.0 && quality != 0  && cnt < POINTS && !lockArrays)
					{
					  //Add received data to the arrays
					  angles[cnt] = f_angle;
					  distances[cnt] = f_distance;

					  //If this is the greatest distance, save it
					  if(f_distance > dmax){
						  dmax = f_distance;
						  amax = f_angle;
					  }
					  else if(f_distance < dmin && f_distance > 0.0f){
						  dmin = f_distance;
						  amin = f_angle;
					  }
					  cnt++;
					}
				}

			}
		}
		k++;
		last_k++;
		next_k++;
		if(k >= 3) k = 0;
		if(last_k >= 3) last_k = 0;
		if(next_k >= 3) next_k = 0;

	}

	received_flag = true;


}
/*
 * //Set up listening for another 5 bytes...
		HAL_UART_Receive_DMA(&huart6, packet, 5);
		 if(!lockArrays)
		 {
			  //Transforming received data
			  uint16_t quality = packet[0] >> 2;
			  uint16_t angle = ((uint16_t)packet[1] >> 1) | (((uint16_t)packet[2]) << 7);
			  float f_angle = ((float) angle) / 64.0; //Q6->float
			  uint16_t distance = ((uint16_t) packet[3]) | ((uint16_t)packet[4] << 8);
			  float f_distance = ((float)distance) / 4.0; //Q2->float

			  if(f_angle > 0.0 && f_angle < 360.0  && cnt < POINTS)
			  {
				  //Add received data to the arrays
				  angles[cnt] = f_angle;
				  distances[cnt] = f_distance;

				  //If this is the greatest distance, save it
				  if(f_distance > dmax)
				  {
					  dmax = f_distance;
					  amax = f_angle;
				  }

				  if(f_distance < dmin && f_distance > 0.0f)
				  {
					  dmin = f_distance;
					  amin = f_angle;
				  }
				  cnt++;
			  }
		  }

	 }
 */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CRC_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  //MX_LTDC_Init();
  MX_QUADSPI_Init();
  MX_SDMMC1_SD_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_TIM12_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

   BSP_LCD_Init();
   BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
   BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS + 1024 * 1024 * 4);
   BSP_LCD_DisplayOn();
   BSP_LCD_SelectLayer(0);


   	//The PWM value is set to 50% duty cycle by default.
   	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);


   	//DMA transmits 2-byte RESET command over UART6
    HAL_UART_Transmit_DMA(&huart6, reset, 2);


    //Clear both layers and wait 1s for the LIDAR to stabilize
 	BSP_LCD_SelectLayer(0);
 	BSP_LCD_Clear(0);
 	BSP_LCD_SelectLayer(1);
 	BSP_LCD_Clear(0);
 	HAL_Delay(1000);

 	//Start-scan request
 	//HAL_UART_Transmit_DMA(&huart6, startscan, 2);

 	//TODO: CHANGE FOR CORRECT 2-BYTE CHECK
 	//Gets one byte from UART until 0x5A packet. 0xA55A is start signal
 	//while(fakerecv != 0x5a)
 	//{
 	//	__HAL_UART_CLEAR_IT(&huart6, UART_CLEAR_NEF|UART_CLEAR_OREF);  //Clear UART cache
 	//	HAL_UART_Receive_DMA(&huart6, &fakerecv, 1); 	//Reveiving a byte will generate interrupt
 	//}



 	startscan_extended[8] = 0x00 ^ 0xa5 ^ 0x82 ^ 0x05 ^ 0x02^ 0x00 ^ 0x00 ^ 0x00 ^ 0x00;


 	HAL_UART_Transmit_DMA(&huart6, startscan_extended, 9);
 	__HAL_UART_CLEAR_IT(&huart6, UART_CLEAR_NEF|UART_CLEAR_OREF);  //Clear UART cache
 	HAL_UART_Receive_DMA(&huart6, buff, 8);
 	while(!received_flag);
 	parseData = true;

	//The DMA controller will now parse the data packets received
 	//This flag also enables continuous receiving.

 	HAL_UART_Receive_DMA(&huart6, buff, BLOCK_SIZE);

 	uint8_t k = 1;

 	while(1)
 	{

 		k^=1;

 		//Wait for data to be gathered..
 		while(cnt < POINTS);

 			//TODO: FIX FLICKERING, SET UP TWO-LAYER DISPLAY MODE
 			BSP_LCD_SetLayerVisible((k^1), ENABLE);
 			HAL_Delay(3);
 			BSP_LCD_SetLayerVisible((k), DISABLE);





			BSP_LCD_SelectLayer(k);
			BSP_LCD_Clear(k);
			draw_grid(COLOR_GRID);
			draw_point(ORIGIN_X, ORIGIN_Y, color(255, 255, 255), 1.0);
			lockArrays = true;
			draw_connected_cloud_fromArray(angles, distances, cnt, amin, dmin, amax, dmax, 0, 0, 1.0, true);
			//draw_cloud_bars_fromArrays(angles, distances, cnt, dmax);
			cnt = 0;
			amax = 0;
			amin = 0;
			dmax = 0;
			dmin = 0;
			lockArrays = false;


			//(Copy layer 0 to layer 1)
			//HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream0, LCD_FB_START_ADDRESS, LCD_FB_START_ADDRESS+1024*1024*4, 480*272*4);
			//BSP_LCD_SetLayerVisible(0, ENABLE);
			//BSP_LCD_SetLayerVisible(1, DISABLE);





 	}

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART6|RCC_PERIPHCLK_SDMMC1
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
