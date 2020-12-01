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


//format: ARGB8888
const uint32_t COLOR_BACKGROUND = 0xff101018;
const uint32_t COLOR_BACKGROUND_GRID = 0xff242430;
const uint32_t COLOR_GRID = 0xff242430;
const uint32_t COLOR_CLOUD0 = 0xff00ffff;
const uint32_t COLOR_CLOUD1 = 0xffff00ff;
const uint32_t COLOR_CLOUD2 = 0xffffff00;




uint32_t color(uint8_t r, uint8_t g, uint8_t b);

void draw_connected_cloud_fromArray(float* angles, float* distances, int size, float amin, float dmin, float amax, float dmax, float scale,  int y_offset, float lightness, bool marks);

float pt_getX(float phi, float dist,  float k);

float pt_getY(float phi, float dist, float k);

void draw_mark( unsigned x, unsigned y, unsigned a, unsigned b, uint32_t c);

void draw_pixel( unsigned x, unsigned y, uint32_t c);

void draw_point( unsigned x, unsigned y, uint32_t c, float lightness = 1.0);
void draw_point( unsigned x, unsigned y, float lightness = 1.0);

void draw_line( float x0, float y0, float x1, float y1, uint32_t c);
void draw_colorful_line(float x0, float y0, float x1, float y1, float dmax);
void draw_ray( float x0, float y0, float x1, float y1, const uint32_t color);

void draw_background( uint32_t c);

void draw_grid( uint32_t c);

void draw_cloud_bars_fromArrays( float * angles, float * distances, int size, float max);

uint32_t calc_color(float v, float lightness = 1.0);

