#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <utility>
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>
#include <rplidar.h>

const int WIDTH = 1280;
const int HEIGHT = 720;
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

struct Cloud {
	std::vector<std::pair<float, float>> pts;
	float min = 25000000;
	float max = 0;
	float avg = 0;
	float std = 0;
	size_t size = 0;
};

//
// Command line arguments parsing
//
bool check_arg_exist(int argc, char** argv, const std::string & arg);

std::string get_arg(int argc, char** argv, const std::string & arg);

void check_invalid_args(int argc, char** argv);

void print_help();

//
// IO
//
bool load_cloud(const std::string& filename, Cloud& cloud);

void load_cloud_from_buffer(rplidar_response_measurement_node_hq_t* buffer, size_t count, Cloud& cloud, bool skip_bad = true);

std::string create_filename(const std::string& dir, const std::string& dot_ext, size_t cnt);

bool save_screenshot(uint8_t* mat, const std::string& dir);

bool save_txt(const Cloud& cloud, const std::string& dir);

//
// Drawing
//
void draw_pixel(uint8_t* mat, int x, int y, color c);

void draw_point(uint8_t* mat, int x, int y, color c, float lightness = 1.0);

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, color c);

void draw_ray(uint8_t* mat, float x0, float y0, float x1, float y1, const color color);

void draw_background(uint8_t* mat, color c);

void draw_grid(uint8_t* mat, color c);

void draw_cloud_bars(uint8_t* mat, const Cloud& cloud);

void draw_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale = 0, int y_offset = 0, float lightness = 1.0, bool marks = false);

color calc_color_angle(float v, float lightness = 1.0);
color calc_color_dist(float dist, float max, float lightness = 1.0);

void draw_mark(uint8_t* mat, unsigned x, unsigned y, unsigned a, unsigned b, color c);

//
// Point cloud calculations
//
std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float k = 1);

float calc_scale(const Cloud& cloud);

void rotate_cloud(Cloud& cloud, float angle);