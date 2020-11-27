#pragma once

#include "main.h"
#include "cmsis_os.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
//#include <SFML\System.hpp>
//#include <SFML\Window.hpp>
//#include <SFML\Graphics.hpp>


const int WIDTH = 480;
const int HEIGHT = 272;
const int CHANNELS = 4;
const int ORIGIN_X = WIDTH / 2;
const int ORIGIN_Y = HEIGHT / 2;


const uint32_t COLOR_BACKGROUND = 0xff101018; 		//color(16, 16, 24);
const uint32_t COLOR_BACKGROUND_GRID = 0xff242430;	//color(36, 36, 48);
const uint32_t COLOR_GRID = 0xff242430; 			//color(36, 36, 48);
const uint32_t COLOR_CLOUD0 = 0xffff7f00;			//color(255, 127, 0);
const uint32_t COLOR_CLOUD1 = 0xff0000ff;			//color(0, 0, 255);
const uint32_t COLOR_CLOUD2 = 0xff00ffc8;			//color(0, 255, 200);


//void load_cloud_fromString(std::string& string, Cloud& cloud, int k, float scale);



// Drawing
//

uint32_t color(uint8_t r, uint8_t g, uint8_t b);

void draw_connected_cloud_fromArray(float* angles, float* distances, int size, float amin, float dmin, float amax, float dmax, float scale,  int y_offset, float lightness, bool marks);

float pt_getX(float phi, float dist,  float k);

float pt_getY(float phi, float dist, float k);

void draw_mark( unsigned x, unsigned y, unsigned a, unsigned b, uint32_t c);

void draw_pixel( unsigned x, unsigned y, uint32_t c);

void draw_point( unsigned x, unsigned y, uint32_t c, float lightness = 1.0);
void draw_point( unsigned x, unsigned y, float lightness = 1.0);

void draw_line( float x0, float y0, float x1, float y1, uint32_t c);
void draw_ray( float x0, float y0, float x1, float y1, const uint32_t color);

void draw_background( uint32_t c);

void draw_grid( uint32_t c);



void draw_cloud_bars_fromArrays( float * angles, float * distances, int size, float max);

uint32_t calc_color(float v, float lightness = 1.0);

//
// Point cloud calculations
//

//float perpendicular_dst(const std::pair<float, float>& pt, const std::pair<float, float>& line_start, const std::pair<float, float>& line_end);

//extern std::string fakefile;
