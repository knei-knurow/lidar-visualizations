#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>

#ifdef WINDOWED_APP
const int WIDTH = 480 * 2;
const int HEIGHT = 272 * 2;
#else
const int WIDTH = 480;
const int HEIGHT = 272;
#endif // WINDOWED_APP

const int CHANNELS = 4;
const int ORIGIN_X = WIDTH / 2;
const int ORIGIN_Y = HEIGHT / 2;

typedef sf::Color color;

const color COLOR_BACKGROUND = color(16, 16, 24);
const color COLOR_BACKGROUND_GRID = color(36, 36, 48);
const color COLOR_GRID = color(36, 36, 48);
const color COLOR_CLOUD0 = color(0, 255, 255);
const color COLOR_CLOUD1 = color(255, 0, 255);
const color COLOR_CLOUD2 = color(255, 255, 0);

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color(uint8_t r, uint8_t g, uint8_t b);
};

struct Cloud {
	std::vector<std::pair<float, float>> pts;
	std::vector<std::vector<std::pair<int, int>>> shape;
	float min = 25000000;
	float max = 0;
	float avg = 0;
	float std = 0;
	size_t size = 0;
};

//
// IO
//
void load_cloud(const std::string& filename, Cloud& cloud, int k, float scale = 0);

bool save_screenshot(uint8_t* mat);
bool save_screenshot(uint8_t* mat, std::string filename);

//
// Drawing
//
void draw_pixel(uint8_t* mat, int x, int y, color c);

void draw_point(uint8_t* mat, int x, int y, color c, float lightness = 1.0);
void draw_point(uint8_t* mat, int x, int y, float lightness = 1.0);

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, color c);

void draw_ray(uint8_t* mat, float x0, float y0, float x1, float y1, const color color);

void draw_background(uint8_t* mat, color c);

void draw_grid(uint8_t* mat, color c);

void draw_cloud_bars(uint8_t* mat, const Cloud& cloud);

void draw_cloud(uint8_t* mat, const Cloud& cloud, float k = 0.04, color c = color::White);

void draw_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale = 0, int y_offset = 0, float lightness = 1.0, bool marks = false);

void draw_cloud_shape(uint8_t* mat, const Cloud& cloud, int y_offset = 0, float lightness = 1.0);

color calc_color(float v, float lightness = 1.0);

void draw_mark(uint8_t* mat, unsigned x, unsigned y, unsigned a, unsigned b, color c);

//
// Point cloud calculations
//
std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float k = 1);

float calc_scale(const Cloud& cloud);

void find_shape(Cloud& cloud, float q = 50, float scale = 0);

void rotate_cloud(Cloud& cloud, float angle);

float perpendicular_dst(const std::pair<float, float>& pt, const std::pair<float, float>& line_start, const std::pair<float, float>& line_end);

void ramer_douglas_peucker(std::vector<std::pair<int, int>> point_list, double epsilon, std::vector<std::pair<int, int>>& out);

void smooth_shape(Cloud& cloud, float epsilon);