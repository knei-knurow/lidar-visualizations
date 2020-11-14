#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
//#include <SFML\System.hpp>
//#include <SFML\Window.hpp>
//#include <SFML\Graphics.hpp>
#include "main.h"
#include "cmsis_os.h"
#include "lidar.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "characters.h"


color::color(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha){
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = alpha;




};


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


void draw_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale, int y_offset, float lightness, bool marks) {
	if (scale == 0)
		scale = calc_scale(cloud);

	if (cloud.size == 0)
		return;

	int cnt = 1;
	auto first_pt = cyl_to_cart(cloud.pts[0], scale);
	auto last_pt = first_pt;
	std::vector<size_t> mark_pt_idx;
	if (cloud.pts[0].second == cloud.max || cloud.pts[0].second == cloud.min)
		mark_pt_idx.push_back(0);
	for (int i = 1; i < cloud.size; i++) {
		auto pt = cyl_to_cart(cloud.pts[i], scale);
		auto c = calc_color(float(cnt) / float(cloud.size), lightness);


		if ((cloud.pts[i].second == cloud.max || cloud.pts[i].second == cloud.min))
			mark_pt_idx.push_back(i);
		if (cloud.pts[i].second > 0 && cloud.pts[i - 1].second > 0)
			draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(pt.first), float(pt.second) + y_offset, c);

		/*
		if (cloud.pts[i].second == cloud.max || cloud.pts[i].second == cloud.min)
		mark_pt_idx.push_back(i);
		draw_line(mat, float(last_pt.first), float(last_pt.second + y_offset), float(pt.first), float(pt.second) + y_offset, c);

		*/
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


void remove_connected_cloud(uint8_t* mat, const Cloud& cloud, float scale, int y_offset, float lightness, bool marks) {
	if (scale == 0)
		scale = calc_scale(cloud);

	if (cloud.size == 0)
		return;

	int cnt = 1;
	auto first_pt = cyl_to_cart(cloud.pts[0], scale);
	auto last_pt = first_pt;
	std::vector<size_t> mark_pt_idx;
	if (cloud.pts[0].second == cloud.max || cloud.pts[0].second == cloud.min)
		mark_pt_idx.push_back(0);
	for (int i = 1; i < cloud.size; i++) {
		auto pt = cyl_to_cart(cloud.pts[i], scale);
		auto c = color(0,0,0);

		if (cloud.pts[i].second == cloud.max || cloud.pts[i].second == cloud.min)
			mark_pt_idx.push_back(i);
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
			draw_mark(mat, pt.first, pt.second + y_offset, unsigned(cloud.pts[i].second / 1000), unsigned(cloud.pts[i].second) % 1000, color(0, 0, 0));
		}
	}
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

void load_cloud_fromString(std::string& string, Cloud& cloud, int k, float scale) {
	std::stringstream fakefile(string);
	while (fakefile) {
		std::string line;
		std::getline(fakefile, line);
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
	//find_shape(cloud, k, scale);
}



void cloud_addPair(Cloud& cloud, float angle, float distance, int k,  float scale) {

	if (distance > cloud.max) cloud.max = distance;
	else if (distance < cloud.min) cloud.min = distance;
	cloud.size++;
	cloud.avg += distance;
	cloud.pts.push_back(std::make_pair(angle, distance));

	cloud.avg /= cloud.size;
	for (auto& pt : cloud.pts)
	{
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}

	cloud.std = std::sqrt(cloud.std);
	//find_shape(cloud, k, scale);
}



//
// Drawing
//

/*
void draw_pixel(uint8_t* mat, unsigned x, unsigned y, color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;
	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}
*/

void draw_pixel(uint8_t* mat, unsigned x, unsigned y, color c)
{

	uint32_t color = (c.a << 24) + (c.r << 16) + (c.g << 8) + c.b;

	if(x <= WIDTH && y <= HEIGHT)
	{
		BSP_LCD_DrawPixel(x, y, color);
	}
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
	uint32_t color = (c.a << 24) + (c.r << 16) + (c.g << 8) + c.b;
	for (int i = 0; i < WIDTH; i++) {
		for(int j = 0; j < HEIGHT; j++)
		{
			BSP_LCD_DrawPixel(i, j, color);
		}
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


void remove_cloud_shape(uint8_t* mat, const Cloud& cloud, int y_offset) {
	std::pair<int, int> pt_prev, pt = cloud.pts.back();
	float dist_prev, dist = cloud.pts.back().second;
	int cnt = 0;
	for (int i = 0; i < cloud.shape.size(); i++) {
		pt_prev = pt;
		dist_prev = dist;
		pt = cloud.shape[i].front();
		draw_point(mat, pt.first, pt.second + y_offset, COLOR_BACKGROUND, 1);
		cnt++;
		for (int j = 1; j < cloud.shape[i].size(); j++) {
			pt_prev = pt;
			pt = cloud.shape[i][j];
			draw_line(mat, pt_prev.first, pt_prev.second + y_offset, pt.first, pt.second + y_offset, COLOR_BACKGROUND);
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



std::string fakefile = " 352.4963 2056.0 \n352.7655 2060.0 \n353.0127 2060.0 \n 353.2654 2068.0 \n353.5291 2068.0 \n353.7762 2064.0 \n354.0289 2064.0 \n354.2926 2068.0 \n354.5453 2072.0 \n354.8090 2076.0 \n355.0616 2076.0 \n355.3088 2072.0 \n355.5780 2072.0 \n355.8252 2076.0 \n356.0779 2080.0 \n356.3416 2080.0 \n356.5887 2084.0 \n356.8414 2084.0 \n357.1051 2088.0 \n357.3578 2096.0 \n357.6215 2096.0 \n357.8741 2096.0 \n358.1213 2100.0 \n358.3905 2104.0 \n358.6377 2104.0 \n358.8904 2104.0 \n359.1541 2112.0 \n359.4012 2116.0 \n359.6539 2116.0 \n359.9176 2124.0 \n0.1703 2120.0 \n0.4340 2124.0 \n0.6866 2132.0 \n0.9338 2136.0 \n1.2030 2132.0 \n1.4502 2136.0 \n1.7029 2144.0 \n1.9666 2144.0 \n2.2137 2144.0 \n2.4664 2148.0 \n2.7301 2156.0 \n2.9828 2160.0 \n3.2465 2160.0 \n3.4991 2164.0 \n3.7463 2172.0 \n4.0155 2172.0 \n4.2627 2180.0 \n4.5154 2184.0 \n4.7791 2184.0 \n5.0262 2188.0 \n5.2954 2196.0 \n5.5426 2200.0 \n5.7953 2200.0 \n6.0590 2204.0 \n6.3116 2204.0 \n6.5588 2212.0 \n6.8280 2220.0 \n7.0752 2228.0 \n7.3279 2228.0 \n7.5916 2232.0 \n7.7948 2240.0 \n8.0420 2244.0 \n8.2947 2248.0 \n8.5583 2252.0 \n8.8110 2256.0 \n9.0582 2268.0 \n9.3274 2268.0 \n9.5746 2272.0 \n9.8273 2280.0 \n10.0909 2284.0 \n10.3436 2292.0 \n10.6073 2300.0 \n10.8545 2300.0 \n11.1072 2308.0 \n11.3708 2316.0 \n11.6235 2316.0 \n11.8707 2324.0 \n12.1399 2328.0 \n12.3871 2328.0 \n12.6398 2336.0 \n12.9034 2344.0 \n13.1561 2356.0 \n13.4198 2360.0 \n13.6670 2360.0 \n13.9197 2368.0 \n14.1833 2376.0 \n14.4360 2384.0 \n14.6997 2388.0 \n14.9524 2396.0 \n15.2161 2400.0 \n15.4688 2408.0 \n15.7324 2416.0 \n15.9796 2424.0 \n16.2323 2432.0 \n16.4960 2440.0 \n16.7487 2448.0 \n17.0123 2456.0 \n17.2156 2460.0 \n17.4683 2464.0 \n17.7319 2476.0 \n17.9791 2480.0 \n18.2483 2488.0 \n18.4955 2496.0 \n18.7482 2504.0 \n19.0118 2516.0 \n19.2645 2524.0 \n19.5282 2528.0 \n19.7809 2540.0 \n20.0446 2552.0 \n20.2917 2560.0 \n20.5609 2564.0 \n20.8081 2572.0 \n21.0773 2584.0 \n21.3245 2596.0 \n21.5936 2600.0 \n21.8408 2608.0 \n22.0935 2620.0 \n22.3572 2628.0 \n22.6044 2636.0 \n22.8735 2648.0 \n23.1207 2664.0 \n23.3899 2676.0 \n23.6371 2688.0 \n23.9063 2692.0 \n24.1534 2708.0 \n24.4171 2716.0 \n24.6698 2728.0 \n24.8730 2740.0 \n25.1202 2752.0 \n25.3894 2760.0 \n25.6366 2776.0 \n25.9058 2788.0 \n26.1530 2792.0 \n26.4166 2804.0 \n26.6693 2816.0 \n26.9330 2824.0 \n27.1857 2840.0 \n27.4329 2856.0 \n27.7020 2868.0 \n27.9492 2876.0 \n28.2184 2888.0 \n28.4656 2912.0 \n28.7292 2928.0 \n29.2456 2940.0 \n29.4983 2836.0 \n29.7620 2828.0 \n30.0146 2816.0 \n30.2618 2796.0 \n30.5310 2796.0 \n30.7782 2804.0 \n31.6077 2724.0 \n31.8713 2716.0 \n32.1240 2712.0 \n32.3877 2728.0 \n33.8104 3248.0 \n34.0741 3272.0 \n34.3268 3272.0 \n34.5905 3292.0 \n34.8431 3312.0 \n35.1068 3324.0 \n35.3540 3348.0 \n35.6232 3372.0 \n35.8704 3392.0 \n36.1230 3400.0 \n36.3867 3416.0 \n36.6394 3444.0 \n36.9031 3460.0 \n37.1558 3484.0 \n37.4194 3504.0 \n37.6227 3516.0 \n38.3862 3548.0 \n38.6389 3616.0 \n38.9026 3664.0 \n39.1553 3676.0 \n39.4025 3704.0 \n39.6716 3732.0 \n39.9188 3760.0 \n40.1715 3780.0 \n40.4352 3804.0 \n40.6824 3772.0 \n40.9515 3728.0 \n41.1987 3744.0 \n41.4514 3772.0 \n59.9030 3492.0 \n60.1666 3480.0 \n60.4193 3472.0 \n60.6665 3512.0 \n60.8698 3572.0 \n61.3861 3676.0 \n62.1991 3152.0 \n62.4683 3180.0 \n62.7155 3212.0 \n62.9791 3204.0 \n63.2318 3184.0 \n63.4955 3156.0 \n63.7482 3128.0 \n63.9954 3124.0 \n64.2645 3124.0 \n64.5117 3120.0 \n64.7809 3116.0 \n65.0281 3116.0 \n65.2917 3120.0 \n65.5444 3124.0 \n66.0608 3168.0 \n66.4343 2656.0 \n66.6870 2620.0 \n66.9507 2628.0 \n67.1979 2636.0 \n67.4670 2644.0 \n67.7142 2656.0 \n67.9669 2668.0 \n68.2306 2680.0 \n68.4833 2692.0 \n68.7469 2700.0 \n68.9996 2712.0 \n69.2633 2724.0 \n69.5105 2732.0 \n69.7137 2744.0 \n69.9829 2752.0 \n70.2301 2764.0 \n70.4828 2776.0 \n70.7465 2784.0 \n70.9991 2788.0 \n71.2628 2804.0 \n71.5155 2820.0 \n71.7792 2832.0 \n72.0264 2844.0 \n72.2955 2856.0 \n72.5427 2868.0 \n72.8119 2884.0 \n73.0591 2896.0 \n73.7457 2054.0 \n74.0588 2032.0 \n74.3115 2030.0 \n74.5752 2026.0 \n74.8279 2026.0 \n75.0916 2026.0 \n75.3387 2026.0 \n75.5914 2024.0 \n75.8551 2024.0 \n76.1078 2024.0 \n76.3715 2024.0 \n76.6241 2024.0 \n76.8878 2024.0 \n77.1405 2024.0 \n77.4042 2024.0 \n77.6514 2024.0 \n77.9205 2026.0 \n78.1677 2024.0 \n78.4369 2024.0 \n78.6841 2026.0 \n78.9368 2026.0 \n79.2004 2024.0 \n79.4531 2026.0 \n79.7168 2026.0 \n79.9640 2026.0 \n80.2332 2026.0 \n80.4803 2022.0 \n80.7495 2024.0 \n80.9967 2028.0 \n81.2604 2028.0 \n81.5131 2028.0 \n81.7603 2028.0 \n82.0294 2030.0 \n82.2766 2030.0 \n82.5458 2032.0 \n82.7930 2032.0 \n83.0621 2034.0 \n83.3093 2034.0 \n83.5730 2034.0 \n83.8257 2036.0 \n84.0894 2036.0 \n84.3420 2036.0 \n84.6057 2038.0 \n84.8584 2040.0 \n85.1056 2040.0 \n85.3748 2042.0 \n85.6219 2044.0 \n85.8856 2046.0 \n86.1383 2044.0 \n86.4020 2048.0 \n86.6547 2048.0 \n86.9183 2048.0 \n87.1216 2052.0 \n87.3743 2052.0 \n87.6215 2056.0 \n87.8906 2056.0 \n88.1378 2060.0 \n88.4015 2060.0 \n88.6542 2060.0 \n88.9178 2060.0 \n89.1705 2064.0 \n89.4177 2064.0 \n89.6869 2068.0 \n89.9341 2072.0 \n90.1868 2072.0 \n90.4504 2076.0 \n90.7031 2076.0 \n90.9668 2080.0 \n91.2140 2080.0 \n91.4667 2084.0 \n91.7303 2084.0 \n91.9830 2088.0 \n92.2467 2092.0 \n92.4994 2092.0 \n92.7466 2096.0 \n93.0103 2100.0 \n93.2629 2100.0 \n93.5266 2104.0 \n93.7793 2104.0 \n94.0265 2108.0 \n94.2957 2112.0 \n94.5428 2112.0 \n94.8120 2116.0 \n95.0592 2120.0 \n95.3119 2124.0 \n95.5756 2124.0 \n95.8228 2128.0 \n96.0919 2132.0 \n96.3391 2136.0 \n96.5918 2140.0 \n96.8555 2144.0 \n97.1082 2148.0 \n97.3718 2148.0 \n97.6245 2152.0 \n97.8717 2156.0 \n98.1354 2160.0 \n98.3881 2164.0 \n98.6517 2168.0 \n98.9044 2172.0 \n99.1516 2176.0 \n99.4208 2180.0 \n99.6680 2184.0 \n99.9371 2188.0 \n100.1843 2192.0 \n100.4370 2200.0 \n100.9479 2128.0 \n101.4642 2216.0 \n101.7169 2224.0 \n101.9806 2224.0 \n102.2333 2224.0 \n102.4805 2232.0 \n103.2001 2236.0 \n103.4528 2248.0 \n103.7164 2256.0 \n103.9636 2260.0 \n104.2328 2264.0 \n104.4800 2272.0 \n104.7327 2276.0 \n104.9963 2288.0 \n105.2490 2284.0 \n105.5127 2264.0 \n105.7654 2244.0 \n106.0730 2224.0 \n106.3257 2204.0 \n106.5894 2176.0 \n106.8420 2160.0 \n107.1057 2148.0 \n107.3584 2132.0 \n107.6056 2112.0 \n107.8748 2092.0 \n108.1219 2076.0 \n108.3856 2056.0 \n108.6987 2040.0 \n108.9514 2022.0 \n109.1986 2006.0 \n109.4678 1990.0 \n109.7150 1972.0 \n109.9841 1956.0 \n110.2313 1940.0 \n110.4840 1926.0 \n110.7477 1912.0 \n110.9949 1898.0 \n111.3080 1886.0 \n111.5607 1874.0 \n111.8243 1860.0 \n112.0770 1846.0 \n112.3407 1834.0 \n112.5934 1820.0 \n112.8406 1810.0 \n113.1042 1796.0 \n113.3569 1780.0 \n113.6206 1768.0 \n113.9337 1756.0 \n114.1864 1744.0 \n114.4336 1732.0 \n114.7028 1718.0 \n114.9500 1706.0 \n115.2026 1694.0 \n115.4663 1684.0 \n115.7135 1672.0 \n115.9827 1662.0 \n116.2299 1650.0 \n116.4825 1640.0 \n116.7957 1630.0 \n117.0593 1618.0 \n117.3120 1610.0 \n117.5592 1600.0 \n117.8229 1586.0 \n118.0756 1578.0 \n118.3228 1570.0 \n118.5919 1562.0 \n118.8391 1550.0 \n119.1083 1546.0 \n119.4214 1536.0 \n119.6686 1528.0 \n119.9213 1518.0 \n120.1849 1512.0 \n120.4321 1508.0 \n120.6848 1498.0 \n120.9485 1490.0 \n121.2012 1486.0 \n121.4648 1476.0 \n121.7175 1468.0 \n121.9647 1460.0 \n122.2339 1452.0 \n122.5415 1446.0 \n122.7942 1440.0 \n123.0469 1438.0 \n123.3105 1430.0 \n123.5577 1422.0 \n123.8269 1418.0 \n124.0741 1408.0 \n124.3268 1400.0 \n124.5905 1394.0 \n124.8431 1390.0 \n125.0903 1386.0 \n125.3540 1380.0 \n125.8704 1376.0 \n126.6229 920.0 \n126.8866 916.0 \n127.1393 912.0 \n127.4524 908.0 \n127.6996 904.0 \n127.9688 902.0 \n128.2159 900.0 \n128.4686 896.0 \n128.7323 892.0 \n128.9795 890.0 \n129.2322 886.0 \n129.4958 882.0 \n130.2649 882.0 \n132.6874 844.0 \n132.9346 846.0 \n133.1873 844.0 \n133.4509 840.0 \n133.6981 838.0 \n133.9508 836.0 \n134.2145 834.0 \n134.4672 832.0 \n134.7308 830.0 \n134.9835 828.0 \n135.2307 824.0 \n135.4999 822.0 \n135.7471 820.0 \n136.0602 818.0 \n136.3074 816.0 \n136.5765 814.0 \n136.8237 814.0 \n137.0764 814.0 \n137.3401 814.0 \n137.5928 816.0 \n137.8564 818.0 \n138.0432 820.0 \n138.3124 820.0 \n138.5596 822.0 \n138.8232 826.0 \n139.0759 828.0 \n139.3396 832.0 \n139.5923 838.0 \n139.8560 844.0 \n140.0427 854.0 \n140.3119 860.0 \n140.5591 868.0 \n140.8228 876.0 \n141.0260 882.0 \n141.2787 892.0 \n141.5259 900.0 \n141.7950 906.0 \n141.9983 916.0 \n142.2455 926.0 \n142.5146 934.0 \n142.7618 942.0 \n143.2178 958.0 \n143.8275 1090.0 \n144.5911 1090.0 \n144.9646 1004.0 \n145.3711 1072.0 \n145.6238 1076.0 \n145.8875 1074.0 \n146.1401 1072.0 \n146.4478 1068.0 \n146.7169 1066.0 \n146.9641 1066.0 \n147.2168 1064.0 \n147.4805 1062.0 \n147.7332 1060.0 \n147.9968 1056.0 \n148.2495 1054.0 \n148.5132 1054.0 \n148.7604 1050.0 \n149.0295 1048.0 \n149.2767 1048.0 \n149.5294 1048.0 \n149.7931 1046.0 \n150.0458 1044.0 \n150.3094 1042.0 \n150.5621 1042.0 \n150.8258 1038.0 \n151.0730 1036.0 \n151.3422 1036.0 \n151.5894 1032.0 \n151.8420 1032.0 \n152.1057 1032.0 \n152.3584 1028.0 \n152.6221 1028.0 \n152.8748 1028.0 \n153.1384 1026.0 \n153.3856 1026.0 \n153.6987 1022.0 \n153.9679 1022.0 \n154.2151 1022.0 \n154.4843 1020.0 \n154.7314 1020.0 \n154.9841 1016.0 \n155.2478 1014.0 \n155.4950 1012.0 \n155.7642 1010.0 \n156.0114 1012.0 \n156.2805 1012.0 \n156.5277 1010.0 \n156.7969 1008.0 \n157.0441 1008.0 \n157.2968 1008.0 \n157.5604 1006.0 \n157.8076 1004.0 \n158.0768 1004.0 \n158.3240 1004.0 \n158.5931 1004.0 \n158.8403 1004.0 \n159.1040 1002.0 \n159.3567 1000.0 \n159.6094 1000.0 \n159.8730 1000.0 \n160.1202 1000.0 \n160.9497 956.0 \n161.1530 992.0 \n161.4166 996.0 \n161.6693 994.0 \n161.9330 992.0 \n162.1857 990.0 \n162.4329 990.0 \n162.7020 990.0 \n162.9492 990.0 \n163.2019 990.0 \n163.4656 992.0 \n163.7183 990.0 \n163.9655 990.0 \n164.2291 990.0 \n164.4818 988.0 \n164.7290 986.0 \n164.9982 986.0 \n165.2454 984.0 \n165.4980 986.0 \n165.7617 988.0 \n166.0144 986.0 \n166.2616 984.0 \n166.5143 986.0 \n166.7780 986.0 \n167.0306 986.0 \n167.2778 984.0 \n167.5415 984.0 \n167.7942 984.0 \n168.0469 986.0 \n168.3105 986.0 \n168.5577 986.0 \n168.8104 986.0 \n169.0741 986.0 \n169.3268 984.0 \n169.5740 984.0 \n169.8431 986.0 \n170.0903 986.0 \n170.3430 984.0 \n170.5902 984.0 \n170.8594 986.0 \n171.1066 984.0 \n171.3593 984.0 \n171.6229 984.0 \n171.8701 986.0 \n172.1228 986.0 \n172.3865 986.0 \n172.6392 984.0 \n172.9523 982.0 \n173.1555 984.0 \n173.4027 988.0 \n173.7158 978.0 \n173.9685 978.0 \n174.6716 988.0 \n190.7007 1034.0 \n191.9806 1046.0 \n192.2498 1060.0 \n192.4969 1066.0 \n192.7606 1066.0 \n193.0133 1068.0 \n193.2166 1072.0 \n193.4637 1074.0 \n193.7329 1074.0 \n193.9801 1078.0 \n194.2493 1080.0 \n194.4965 1082.0 \n194.7656 1084.0 \n195.0128 1088.0 \n195.2655 1090.0 \n195.5292 1092.0 \n195.7764 1094.0 \n196.0455 1096.0 \n196.2927 1098.0 \n196.5619 1100.0 \n196.8091 1104.0 \n197.0618 1106.0 \n197.3254 1110.0 \n197.5781 1112.0 \n197.8418 1114.0 \n198.0450 1118.0 \n198.2922 1120.0 \n198.5449 1122.0 \n198.8086 1124.0 \n199.0613 1128.0 \n199.3250 1130.0 \n199.5776 1132.0 \n199.8248 1136.0 \n200.0885 1140.0 \n200.3412 1142.0 \n200.6049 1144.0 \n200.8575 1148.0 \n201.1212 1152.0 \n201.3739 1154.0 \n201.6211 1156.0 \n201.8903 1160.0 \n202.1375 1162.0 \n202.4011 1166.0 \n202.6538 1170.0 \n202.8571 1174.0 \n203.1042 1176.0 \n203.3734 1180.0 \n203.6206 1184.0 \n203.8898 1188.0 \n204.1370 1192.0 \n204.4061 1196.0 \n204.6533 1200.0 \n204.9060 1202.0 \n205.1697 1206.0 \n205.4169 1210.0 \n205.6860 1214.0 \n205.9332 1218.0 \n206.2024 1222.0 \n206.4496 1226.0 \n206.6528 1230.0 \n206.9055 1236.0 \n207.1692 1238.0 \n207.4219 1242.0 \n207.6855 1246.0 \n207.9327 1252.0 \n208.1854 1256.0 \n208.4491 1262.0 \n209.0149 1216.0 \n209.2786 1204.0 \n209.5313 1200.0 \n209.7949 1192.0 \n210.0421 1186.0 \n210.2948 1180.0 \n210.6079 1170.0 \n210.8716 1156.0 \n211.1243 1154.0 \n211.3879 1158.0 \n212.1515 1142.0 \n212.4042 1128.0 \n212.9205 1126.0 \n213.7500 1090.0 \n213.9972 1104.0 \n214.2609 1100.0 \n214.5135 1098.0 \n214.7607 1092.0 \n215.0299 1086.0 \n215.2771 1082.0 \n215.5463 1078.0 \n215.7935 1074.0 \n216.1066 1068.0 \n216.3593 1064.0 \n216.6229 1060.0 \n216.8701 1056.0 \n217.1393 1052.0 \n217.3865 1048.0 \n217.6392 1042.0 \n217.9028 1038.0 \n218.1555 1036.0 \n218.4192 1032.0 \n218.6719 1028.0 \n218.9795 1024.0 \n219.1827 1026.0 \n219.4958 1020.0 \n219.7485 1010.0 \n220.0122 1004.0 \n220.3253 982.0 \n220.8252 982.0 \n221.0284 996.0 \n221.2921 994.0 \n221.5887 982.0 \n221.8579 978.0 \n222.3083 984.0 \n223.1378 976.0 \n223.3905 958.0 \n223.6981 934.0 \n223.9673 922.0 \n224.2145 922.0 \n224.4672 934.0 \n224.6704 948.0 \n224.9341 948.0 \n225.1868 946.0 \n225.4999 944.0 \n225.7471 940.0 \n226.0107 938.0 \n226.2634 936.0 \n226.5271 934.0 \n226.7798 930.0 \n227.0270 928.0 \n227.2961 926.0 \n227.5433 922.0 \n227.8125 920.0 \n228.0597 918.0 \n228.3124 916.0 \n228.5760 912.0 \n228.8892 910.0 \n229.1364 910.0 \n229.4055 906.0 \n229.6527 904.0 \n229.9054 900.0 \n230.1691 900.0 \n230.4218 898.0 \n230.6689 896.0 \n230.9326 894.0 \n231.8390 960.0 \n232.1082 964.0 \n232.3553 970.0 \n232.6080 976.0 \n232.8717 982.0 \n233.0750 990.0 \n233.3276 996.0 \n233.5748 1004.0 \n233.8385 1008.0 \n234.0912 1018.0 \n234.2944 1026.0 \n234.5416 1032.0 \n234.8108 1040.0 \n235.0580 1048.0 \n235.3271 1054.0 \n235.5743 1062.0 \n235.7776 1070.0 \n236.0303 1082.0 \n236.2939 1088.0 \n236.5466 1096.0 \n236.7938 1104.0 \n237.0575 1112.0 \n237.2498 1120.0 \n237.5134 1130.0 \n237.7606 1138.0 \n238.0133 1148.0 \n238.2770 1158.0 \n238.5297 1168.0 \n238.7329 1178.0 \n238.9801 1188.0 \n239.2328 1198.0 \n239.4965 1210.0 \n239.7491 1220.0 \n240.6555 1350.0 \n240.9027 1358.0 \n241.1719 1364.0 \n241.3586 1370.0 \n241.6223 1374.0 \n241.8750 1374.0 \n242.1222 1376.0 \n242.3859 1376.0 \n242.6385 1376.0 \n242.8857 1374.0 \n243.1384 1374.0 \n243.4021 1372.0 \n243.6548 1370.0 \n243.9020 1368.0 \n244.1711 1368.0 \n244.4183 1368.0 \n244.6710 1370.0 \n244.9347 1374.0 \n245.1874 1382.0 \n245.4346 1390.0 \n245.6982 1390.0 \n245.9509 1384.0 \n246.1981 1382.0 \n246.4673 1380.0 \n246.7145 1380.0 \n246.9672 1382.0 \n247.2144 1384.0 \n247.4835 1400.0 \n247.7307 1416.0 \n247.9834 1416.0 \n248.2471 1418.0 \n248.4998 1422.0 \n248.7469 1444.0 \n248.9502 1466.0 \n249.2029 1472.0 \n249.4666 1476.0 \n249.7137 1478.0 \n249.9664 1480.0 \n250.2301 1480.0 \n250.4828 1480.0 \n250.7300 1480.0 \n250.9991 1478.0 \n251.2463 1476.0 \n251.4990 1476.0 \n251.7627 1476.0 \n252.0154 1476.0 \n252.2626 1478.0 \n252.5153 1482.0 \n252.7789 1496.0 \n253.0261 1522.0 \n253.2788 1534.0 \n253.5425 1532.0 \n253.7952 1530.0 \n254.0424 1530.0 \n254.3115 1532.0 \n254.5587 1536.0 \n254.7620 1552.0 \n255.2179 1648.0 \n255.4651 1646.0 \n255.7178 1646.0 \n255.9814 1646.0 \n256.2341 1644.0 \n256.4813 1644.0 \n256.7450 1646.0 \n256.9977 1646.0 \n257.2449 1648.0 \n257.5140 1650.0 \n257.7612 1652.0 \n258.0139 1656.0 \n258.5742 1590.0 \n258.8434 1584.0 \n259.0906 1592.0 \n259.3433 1602.0 \n259.6069 1612.0 \n259.8541 1622.0 \n260.1068 1630.0 \n260.3705 1636.0 \n260.5737 1642.0 \n260.8264 1644.0 \n261.0736 1648.0 \n261.3428 1652.0 \n261.5900 1656.0 \n261.8591 1656.0 \n262.1063 1656.0 \n262.3700 1658.0 \n262.6227 1660.0 \n262.8699 1662.0 \n263.1390 1666.0 \n263.3862 1670.0 \n263.9026 1678.0 \n264.6222 1830.0 \n270.6866 219.0 \n270.9338 218.0 \n271.2469 217.0 \n271.5106 217.0 \n271.7633 216.0 \n272.0764 215.0 \n272.3401 215.0 \n272.5928 215.0 \n272.8564 215.0 \n273.1091 215.0 \n273.3563 215.0 \n273.6200 215.0 \n273.8727 215.0 \n274.1364 215.0 \n274.3890 214.0 \n274.6527 214.0 \n274.9054 214.0 \n275.1526 214.0 \n275.4218 214.0 \n275.6689 214.0 \n275.9326 214.0 \n276.1853 214.0 \n276.4490 215.0 \n276.7017 215.0 \n276.9489 215.0 \n277.1521 216.0 \n277.4213 216.0 \n277.6685 217.0 \n277.9211 217.0 \n278.1848 216.0 \n278.4375 217.0 \n278.7012 217.0 \n278.9484 217.0 \n279.1516 218.0 \n279.4043 218.0 \n279.6680 218.0 \n279.9207 219.0 \n280.1843 219.0 \n280.4370 219.0 \n280.7007 219.0 \n280.8875 220.0 \n281.1511 220.0 \n281.4038 220.0 \n281.6675 220.0 \n281.9202 220.0 \n282.1674 220.0 \n282.4365 221.0 \n282.6837 221.0 \n282.9529 221.0 \n283.2001 221.0 \n283.4637 221.0 \n283.7164 221.0 \n283.9636 221.0 \n284.2328 221.0 \n284.4800 221.0 \n284.7491 221.0 \n284.9963 220.0 \n285.2655 220.0 \n285.5731 219.0 \n285.8258 218.0 \n286.0895 218.0 \n286.3422 218.0 \n286.5894 218.0 \n286.8585 218.0 \n287.1057 219.0 \n287.3749 219.0 \n287.6221 219.0 \n287.8253 220.0 \n288.0780 220.0 \n288.3417 221.0 \n288.5449 222.0 \n288.7921 222.0 \n289.0448 223.0 \n289.2480 224.0 \n289.5117 225.0 \n289.7644 225.0 \n289.9677 226.0 \n290.2148 226.0 \n290.4840 227.0 \n290.7312 227.0 \n290.9344 228.0 \n291.1871 229.0 \n291.3904 230.0 \n291.6376 230.0 \n291.8408 232.0 \n292.0935 233.0 \n292.7472 237.0 \n292.8900 242.0 \n293.0768 244.0 \n293.3405 244.0 \n293.5931 245.0 \n293.7964 246.0 \n294.0436 248.0 \n294.2963 248.0 \n294.4995 249.0 \n294.7467 250.0 \n295.4663 253.0 \n296.0266 304.0 \n296.2958 305.0 \n296.5430 306.0 \n296.7957 307.0 \n296.9989 308.0 \n297.2461 309.0 \n297.5153 309.0 \n297.7625 311.0 \n297.9657 313.0 \n298.2184 314.0 \n298.4656 315.0 \n298.7183 315.0 \n298.9215 317.0 \n299.2291 314.0 \n300.1355 289.0 \n300.4486 285.0 \n300.6354 286.0 \n300.9045 286.0 \n301.1517 286.0 \n301.4044 286.0 \n301.6681 286.0 \n301.9208 286.0 \n302.1680 287.0 \n302.4371 288.0 \n302.6239 290.0 \n302.8876 290.0 \n303.1403 290.0 \n303.3875 291.0 \n303.6401 292.0 \n303.8434 293.0 \n304.7498 303.0 \n304.9530 304.0 \n305.2002 304.0 \n305.4639 304.0 \n305.7166 304.0 \n305.9637 304.0 \n306.2164 304.0 \n306.4801 305.0 \n306.7328 306.0 \n306.9800 306.0 \n307.1832 308.0 \n307.4359 309.0 \n307.6996 311.0 \n308.1555 313.0 \n308.6829 331.0 \n308.9355 332.0 \n309.1388 334.0 \n309.3860 336.0 \n309.5892 337.0 \n309.8419 337.0 \n310.1056 337.0 \n310.3583 340.0 \n310.5615 343.0 \n310.8087 344.0 \n311.0614 344.0 \n311.3251 344.0 \n311.5778 345.0 \n311.8250 346.0 \n312.0886 346.0 \n312.4017 340.0 \n313.1982 359.0 \n313.4509 360.0 \n313.7146 361.0 \n313.9673 359.0 \n314.6539 343.0 \n314.9011 342.0 \n315.1703 342.0 \n315.4834 341.0 \n315.7306 341.0 \n315.9833 341.0 \n316.2469 340.0 \n316.4996 338.0 \n316.8073 334.0 \n317.1863 327.0 \n317.4994 323.0 \n317.7466 323.0 \n318.0103 322.0 \n318.2629 321.0 \n318.5156 320.0 \n318.7793 320.0 \n319.0924 319.0 \n319.3396 318.0 \n319.6088 316.0 \n319.9219 314.0 \n320.1691 313.0 \n320.4822 311.0 \n320.7458 311.0 \n320.9985 310.0 \n321.2457 310.0 \n321.5149 310.0 \n321.7621 310.0 \n322.0313 310.0 \n322.2784 310.0 \n322.5421 310.0 \n322.7948 309.0 \n323.0420 310.0 \n323.3112 311.0 \n323.5583 311.0 \n324.2780 312.0 \n324.5306 315.0 \n324.7339 318.0 \n324.9371 320.0 \n325.2448 319.0 \n325.4480 320.0 \n325.7007 320.0 \n326.0138 319.0 \n326.2775 319.0 \n326.5302 319.0 \n326.7334 320.0 \n326.9806 320.0 \n327.2498 321.0 \n327.4969 321.0 \n327.7496 321.0 \n328.0133 322.0 \n328.2605 323.0 \n328.4637 324.0 \n328.7329 325.0 \n328.9801 326.0 \n329.2328 326.0 \n329.4965 327.0 \n329.6997 328.0 \n329.9524 330.0 \n330.1996 331.0 \n330.4028 333.0 \n330.6665 334.0 \n330.9192 336.0 \n331.1224 338.0 \n331.3696 341.0 \n331.5729 343.0 \n332.2925 347.0 \n334.6216 2070.0 \n334.9347 2046.0 \n335.1984 2042.0 \n335.4510 2040.0 \n335.6982 2040.0 \n335.9674 2038.0 \n336.2146 2034.0 \n336.4838 2036.0 \n336.7310 2036.0 \n336.9836 2034.0 \n337.2473 2034.0 \n337.5000 2034.0 \n337.7472 2034.0 \n338.0109 2034.0 \n338.2635 2034.0 \n338.5107 2030.0 \n338.7799 2028.0 \n339.0271 2026.0 \n339.2963 2026.0 \n339.5435 2028.0 \n339.7961 2030.0 \n340.0598 2030.0 \n340.3125 2024.0 \n340.5597 2022.0 \n340.8234 2028.0 \n341.0760 2030.0 \n341.3232 2026.0 \n341.5924 2022.0 \n341.8396 2026.0 \n342.1088 2026.0 \n342.3560 2026.0 \n342.6086 2028.0 \n342.8723 2030.0 \n343.1250 2026.0 \n343.3722 2024.0 \n343.6359 2024.0 \n343.8885 2028.0 \n344.1357 2030.0 \n344.4049 2028.0 \n344.6521 2030.0 \n344.9213 2030.0 \n345.1685 2030.0 \n345.4211 2028.0 \n345.6848 2026.0 \n345.9375 2030.0 \n346.1847 2034.0 \n346.4484 2032.0 \n346.7010 2032.0 \n346.9482 2032.0 \n347.2174 2034.0 \n347.4646 2036.0 \n347.7338 2038.0 \n347.9810 2036.0 \n348.2336 2036.0 \n348.4973 2040.0 \n348.7500 2040.0 \n348.9972 2038.0 \n349.2609 2044.0 \n349.5135 2046.0 \n349.7607 2044.0 \n350.0299 2040.0 \n350.2771 2044.0 \n350.5463 2048.0 \n350.7935 2048.0 \n351.0461 2048.0 \n351.3098 2048.0 \n351.4966 2056.0 \n351.7603 2052.0 \n352.0129 2052.0 \n352.2766 2056.0";
