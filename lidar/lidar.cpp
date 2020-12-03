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
#include "characters.h"
#include <rplidar.h>

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
		if (dist < cloud.min && dist > 0) cloud.min = dist;
		cloud.size++;
		cloud.avg += dist;
		cloud.pts.push_back(std::make_pair(angle, dist));
	}
	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);
	// find_shape(cloud, k, scale);
}

void load_cloud_from_buffer(rplidar_response_measurement_node_hq_t* buffer, size_t count, Cloud& cloud, bool skip_bad) {
	cloud = Cloud();
	for (int i = 0; i < count; i++) {
		float angle = buffer[i].angle_z_q14 / 65536.0f * 360;
		float dist = buffer[i].dist_mm_q2 / 4.0f;

		if (skip_bad && dist == 0) continue;
		if (dist > cloud.max) cloud.max = dist;
		if (dist < cloud.min && dist > 0) cloud.min = dist;
		cloud.size++;
		cloud.avg += dist;
		cloud.pts.push_back(std::make_pair(angle, dist));
	}
	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);
}

bool save_screenshot(uint8_t* mat, std::string filename) {
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	texture.update(mat);
	return texture.copyToImage().saveToFile(filename);
}

bool save_screenshot(uint8_t* mat) {
	static size_t cnt = 0;
	return save_screenshot(mat, std::to_string(time(0)) + "-" + std::to_string(cnt++) + ".png");
}

bool save_txt(const Cloud& cloud, const std::string& filename) {
	std::ofstream file(filename);
	if (!file) {
		return false;
	}
	file << "# RPLIDAR SCAN DATA" << std::endl;
	file << "# Software: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
	file << "# Authors: Szymon Bednorz, Bartek Dudek" << std::endl;
	file << "# Angle Distance" << std::endl;
	for (const auto& pt : cloud.pts) {
		file << pt.first << " " << pt.second << std::endl;
	}
	return true;
}

bool save_txt(const Cloud& cloud) {
	static size_t cnt = 0;
	return save_txt(cloud, std::to_string(time(0)) + "-" + std::to_string(cnt++) + ".txt");
}

//
// Drawing
//
void draw_pixel(uint8_t* mat, int x, int y, color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;
	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

void draw_point(uint8_t* mat, int x, int y, color c, float lightness) {
	c.r *= lightness;
	c.g *= lightness;
	c.b *= lightness;
	for (auto cx : { -1, 0, 1 }) {
		for (auto cy : { -1, 0, 1 }) {
			draw_pixel(mat, x + cx, y + cy, c);
		}
	}
}

void draw_point(uint8_t* mat, int x, int y, float lightness) {
	color c = calc_color(float(x), float(y));
	draw_point(mat, x, y, c, lightness);
}

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, color c) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		draw_point(mat, int(x0), int(y0), c);
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
	if (cloud.size == 0) return;
	unsigned max_width = 80;
	for (int j = 0; j < HEIGHT; j++) {
		float dist = cloud.pts[size_t(j * cloud.size / HEIGHT)].second;

		int width = int(std::round(dist / cloud.max * max_width));

		color c = calc_color(float(j * cloud.size / HEIGHT) / float(cloud.size));
		for (int i = 0; i < width; i++) {
			draw_pixel(mat, i, j, c);
		}
	}
}

void draw_cloud(uint8_t* mat, const Cloud& cloud, float scale, color c) {
	for (auto & pt : cloud.pts) {
		auto ptc = cyl_to_cart(pt, scale);
		draw_point(mat, ptc.first, ptc.second);
	}
}

void draw_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale, int y_offset, float lightness, bool marks) {
	if (scale == 0)
		scale = calc_scale(cloud);

	if (cloud.size == 0)
		return;

	int cnt = 1;
	auto first_pt = cyl_to_cart(cloud.pts[0], scale);
	auto last_pt = first_pt;
	std::vector<size_t> mark_pt_idx;
	//if (cloud.pts[0].second == cloud.max || cloud.pts[0].second == cloud.min)
	//	mark_pt_idx.push_back(0);
	for (int i = 1; i < cloud.size; i++) {
		auto pt = cyl_to_cart(cloud.pts[i], scale);
		auto c = calc_color(float(cnt) / float(cloud.size), lightness);

		/*if ((cloud.pts[i].second == cloud.max || cloud.pts[i].second == cloud.min))
			mark_pt_idx.push_back(i);*/
		if (cloud.pts[i].second > 0 && cloud.pts[i - 1].second > 0)
			draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(pt.first), float(pt.second) + y_offset, c);

		last_pt = pt;
		cnt++;
	}
	auto c = calc_color(float(cnt) / float(cloud.size), lightness);
	draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(first_pt.first), float(first_pt.second + y_offset), c);

	if (marks) {
		for (auto i : mark_pt_idx) {
			auto pt = cyl_to_cart(cloud.pts[i], scale);
			draw_point(mat, pt.first, pt.second + y_offset, color(255, 255, 255));
			draw_mark(mat, pt.first, pt.second + y_offset, unsigned(cloud.pts[i].second / 1000), unsigned(cloud.pts[i].second) % 1000, color(255, 255, 255));
		}
	}
}

void draw_cloud_shape(uint8_t* mat, const Cloud& cloud, int y_offset, float lightness) {
	std::pair<float, float> pt_prev, pt = cloud.pts.back();
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
			draw_line(mat, float(pt_prev.first), float(pt_prev.second + y_offset), float(pt.first), float(pt.second + y_offset), calc_color(float(cnt) / float(cloud.size), lightness));
			cnt++;
		}
	}
}

color calc_color(float v, float lightness) {
	color c0, c1;
	if (v >= 0 && v <= 0.33f) {
		c0 = COLOR_CLOUD0;
		c1 = COLOR_CLOUD1;
	}
	else if (v <= 0.66f) {
		v -= 0.33f;
		c0 = COLOR_CLOUD2;
		c1 = COLOR_CLOUD0;
	}
	else if (v <= 1.0f) {
		v -= 0.66f;
		c0 = COLOR_CLOUD1;
		c1 = COLOR_CLOUD2;
	}
	else {
		return color(255, 255, 255);
	}

	c0.r *= float(v) / 0.34f;
	c0.g *= float(v) / 0.34f;
	c0.b *= float(v) / 0.34f;
	c1.r *= 1.0f - float(v) / 0.34f;
	c1.g *= 1.0f - float(v) / 0.34f;
	c1.b *= 1.0f - float(v) / 0.34f;
	return color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness, float(c0.b + c1.b) * lightness);
}

void draw_mark(uint8_t* mat, unsigned x, unsigned y, unsigned a, unsigned b, color c) {
	draw_point(mat, x, y, c);
	auto str = std::to_string(a) + "." + std::to_string(b);
	x += -12;
	y += 5;
	for (int ch : str) {
		if (ch == '.') ch = CHAR_DOT;
		else ch -= '0';

		for (int cy = 0; cy < CHAR_HEIGHT; cy++) {
			for (int cx = 0; cx < CHAR_MAT[ch][cy].size(); cx++) {
				if (CHAR_MAT[ch][cy][cx] == '#')
					draw_pixel(mat, x + cx, y + cy, c);
			}
		}
		x += CHAR_MAT[ch][0].size() + 1;
	}
}

//
// Point cloud calculations
//
std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float scale) {
	float phi = pt.first;
	float dist = pt.second;
	int x = int(std::round(dist * std::sin(phi * (acos(-1.0f) / 180.0f)) * scale)) + ORIGIN_X;
	int y = int(std::round(dist * std::cos(phi * (acos(-1.0f) / 180.0f)) * scale)) + ORIGIN_Y;
	return std::make_pair(x, y);
}

float calc_scale(const Cloud& cloud) {
	return float(HEIGHT) * 0.7f / cloud.max;
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

	float mag = std::pow(std::pow(dx, 2.0f) + std::pow(dy, 2.0f), 0.5f);
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

	return std::pow(std::pow(ax, 2.0f) + std::pow(ay, 2.0f), 0.5f);
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
