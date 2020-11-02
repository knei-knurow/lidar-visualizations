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
#include "lidar.h"

Color::Color(uint8_t r, uint8_t g, uint8_t b) {
	this->r = r;
	this->g = g;
	this->g = g;
}

//
// IO
//
void load_cloud(const std::string& filename, Cloud& cloud, int k, float scale) {
	std::ifstream file(filename);
	while (file) {
		std::string line;
		std::getline(file, line);
		if (line.empty() || line[0] == '#')
			continue;
		float angle, dist;
		std::stringstream sline(line);
		sline >> angle >> dist;

		if (dist > cloud.max) cloud.max = dist;
		else if (dist < cloud.min) cloud.min = dist;
		cloud.size++;
		cloud.avg += dist;
		cloud.pts.push_back(std::make_pair(angle, dist));
	}
	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);
	find_shape(cloud, k, scale);
}

bool save_screenshot(uint8_t* mat, std::string filename) {
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	texture.update(mat);
	return texture.copyToImage().saveToFile(filename);
}

bool save_screenshot(uint8_t* mat) {
	return save_screenshot(mat, std::to_string(time(0)) + ".png");
}

//
// Drawing
//
void draw_pixel(uint8_t* mat, unsigned x, unsigned y, color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;
	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

void draw_point(uint8_t* mat, unsigned x, unsigned y, color c, float lightness) {
	c.r *= lightness;
	c.g *= lightness;
	c.b *= lightness;
	for (auto cx : { -1, 0, 1 }) {
		for (auto cy : { -1, 0, 1 }) {
			draw_pixel(mat, x + cx, y + cy, c);
		}
	}
}

void draw_point(uint8_t* mat, unsigned x, unsigned y, float lightness) {
	color c = calc_color(x, y);
	draw_point(mat, x, y, c, lightness);
}

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, color c) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		draw_point(mat, x0, y0, c);
		x0 += x; y0 += y;
	}
}

void draw_ray(uint8_t* mat, float x0, float y0, float x1, float y1, color c) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	while (x0 < WIDTH && x0 >= 0 && y0 < HEIGHT && y0 >= 0) {
		draw_point(mat, x0, y0, c);
		x0 += x; y0 += y;
	}
}

void draw_background(uint8_t* mat, color c) {
	for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i += CHANNELS) {
		mat[i + 0] = c.r;
		mat[i + 1] = c.g;
		mat[i + 2] = c.b;
		mat[i + 3] = c.a;
	}
}

void draw_grid(uint8_t* mat, color c) {
	for (int x = 0; x < WIDTH; x += WIDTH / 8) {
		for (int y = 0; y < HEIGHT; y++) {
			draw_pixel(mat, x, y, c);
		}
	}
	for (int y = 0; y < HEIGHT; y += HEIGHT / 8) {
		for (int x = 0; x < WIDTH; x++) {
			draw_pixel(mat, x, y, c);
		}
	}
}

void draw_cloud_bars(uint8_t* mat, const Cloud& cloud) {
	unsigned max_width = 80;
	for (int j = 0; j < HEIGHT; j++) {
		float dist = cloud.pts[size_t(j * cloud.size / HEIGHT)].second;

		unsigned width = unsigned(std::round(dist / cloud.max * max_width));

		for (int i = 0; i < width; i++) {
			draw_pixel(mat, i, j, calc_color(float(j * cloud.size / HEIGHT) / float(cloud.size)));
		}
	}
}

void draw_cloud(uint8_t* mat, const Cloud& cloud, float k, color c) {
	for (auto& pt : cloud.pts) {
		float phi = pt.first;
		float dist = pt.second;
		int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + ORIGIN_X;
		int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + ORIGIN_Y;

		draw_point(mat, x, y);
	}
}

void draw_cloud_shape(uint8_t* mat, const Cloud& cloud, int y_offset, float lightness) {
	std::pair<int, int> pt_prev, pt = cloud.pts.back();
	float dist_prev, dist = cloud.pts.back().second;
	int cnt = 0;
	for (int i = 0; i < cloud.shape.size(); i++) {
		pt_prev = pt;
		dist_prev = dist;
		pt = cloud.shape[i].front();
		draw_point(mat, pt.first, pt.second + y_offset, calc_color(float(cnt) / float(cloud.size), lightness));
		cnt++;
		for (int j = 1; j < cloud.shape[i].size(); j++) {
			pt_prev = pt;
			pt = cloud.shape[i][j];
			draw_line(mat, pt_prev.first, pt_prev.second + y_offset, pt.first, pt.second + y_offset, calc_color(float(cnt) / float(cloud.size), lightness));
			cnt++;
		}
	}
}

color calc_color(float v, float lightness) {
	if (v >= 0 && v <= 0.33f) {
		color c0 = COLOR_CLOUD0;
		color c1 = COLOR_CLOUD1;
		c0.r *= v / 0.34f;
		c0.g *= v / 0.34f;
		c0.b *= v / 0.34f;
		c1.r *= 1.0 - v / 0.34f;
		c1.g *= 1.0 - v / 0.34f;
		c1.b *= 1.0 - v / 0.34f;
		return color((c0.r + c1.r) * lightness, (c0.g + c1.g) * lightness, (c0.b + c1.b) * lightness);
	}
	else if (v <= 0.66f) {
		v -= 0.33;
		color c0 = COLOR_CLOUD2;
		color c1 = COLOR_CLOUD0;
		c0.r *= v / 0.34f;
		c0.g *= v / 0.34f;
		c0.b *= v / 0.34f;
		c1.r *= 1.0 - v / 0.34f;
		c1.g *= 1.0 - v / 0.34f;
		c1.b *= 1.0 - v / 0.34f;
		return color((c0.r + c1.r) * lightness, (c0.g + c1.g) * lightness, (c0.b + c1.b) * lightness);
	}
	else if (v <= 1.0f) {
		v -= 0.66;
		color c0 = COLOR_CLOUD1;
		color c1 = COLOR_CLOUD2;
		c0.r *= v / 0.34f;
		c0.g *= v / 0.34f;
		c0.b *= v / 0.34f;
		c1.r *= 1.0 - v / 0.34f;
		c1.g *= 1.0 - v / 0.34f;
		c1.b *= 1.0 - v / 0.34f;
		return color((c0.r + c1.r) * lightness, (c0.g + c1.g) * lightness, (c0.b + c1.b) * lightness);
	}
	else {
		return color(255 * lightness, 255 * lightness, 255 * lightness);
	}
}

//
// Point cloud calculations
//
std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float k) {
	float phi = pt.first;
	float dist = pt.second;
	int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + ORIGIN_X;
	int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + ORIGIN_Y;
	return std::make_pair(x, y);
}

float calc_scale(const Cloud& cloud) {
	return float(HEIGHT) * 0.7 / cloud.max;
}

void find_shape(Cloud& cloud, float q, float scale) {
	if (scale == 0)
		scale = calc_scale(cloud);
	cloud.shape = { {cyl_to_cart(cloud.pts[0], scale)} };
	for (size_t i = 1; i < cloud.size; i++) {
		float dist0 = cloud.pts[i - 1].second;
		float dist1 = cloud.pts[i].second;

		if (std::fabs(dist0 - dist1) <= q) {
			cloud.shape.back().push_back(cyl_to_cart(cloud.pts[i], scale));
		}
		else {
			cloud.shape.push_back({ cyl_to_cart(cloud.pts[i], scale) });
		}
	}
}

void rotate_cloud(Cloud& cloud, float angle) {
	for (auto& i : cloud.pts) {
		i.first += angle;
		if (i.first >= 360) i.first -= 360;
	}
}

float perpendicular_dst(const std::pair<float, float>& pt, const std::pair<float, float>& line_start, const std::pair<float, float>& line_end) {
	float dx = line_end.first - line_start.first;
	float dy = line_end.second - line_start.second;

	float mag = std::pow(std::pow(dx, 2.0) + std::pow(dy, 2.0), 0.5);
	if (mag > 0.0) {
		dx /= mag; dy /= mag;
	}

	float pvx = pt.first - line_start.first;
	float pvy = pt.second - line_start.second;

	float pvdot = dx * pvx + dy * pvy;

	float dsx = pvdot * dx;
	float dsy = pvdot * dy;

	float ax = pvx - dsx;
	float ay = pvy - dsy;

	return std::pow(std::pow(ax, 2.0) + std::pow(ay, 2.0), 0.5);
}

void ramer_douglas_peucker(std::vector<std::pair<int, int>> point_list, double epsilon, std::vector<std::pair<int, int>>& out)
{
	if (point_list.size() < 2)
		return;

	float dmax = 0.0;
	size_t index = 0;
	size_t end = point_list.size() - 1;
	for (size_t i = 1; i < end; i++) {
		float d = perpendicular_dst(point_list[i], point_list[0], point_list[end]);
		if (d > dmax) {
			index = i;
			dmax = d;
		}
	}

	if (dmax > epsilon) {
		std::vector<std::pair<int, int>> recResults1;
		std::vector<std::pair<int, int>> recResults2;
		std::vector<std::pair<int, int>> firstLine(point_list.begin(), point_list.begin() + index + 1);
		std::vector<std::pair<int, int>> lastLine(point_list.begin() + index, point_list.end());
		ramer_douglas_peucker(firstLine, epsilon, recResults1);
		ramer_douglas_peucker(lastLine, epsilon, recResults2);

		out.assign(recResults1.begin(), recResults1.end() - 1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if (out.size() < 2)
			return;
	}
	else {
		out.clear();
		out.push_back(point_list[0]);
		out.push_back(point_list[end]);
	}
}

void smooth_shape(Cloud& cloud, float epsilon) {
	for (auto& shape : cloud.shape) {
		ramer_douglas_peucker(shape, epsilon, shape);
	}
}
