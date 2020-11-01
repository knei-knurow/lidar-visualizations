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

const int WIDTH = 480 * 2;
const int HEIGHT = 272 * 2;
const int CHANNELS = 4;
int ORIGIN_X = WIDTH / 2;
int ORIGIN_Y = HEIGHT / 2 - HEIGHT / 3;
const std::string TITLE = "STM32F746G-DISCO | 480x272";
const sf::Uint32 STYLE = sf::Style::Close | sf::Style::Titlebar;
sf::RenderWindow window_;
bool running_ = true;

struct Cloud {
	std::vector<std::pair<float, float>> pts;
	std::vector<std::vector<std::pair<int, int>>> shape;
	float min = std::numeric_limits<float>::infinity();
	float max = 0;
	float avg = 0;
	float std = 0;
	size_t size = 0;
};

std::pair<int, int> cyl_to_cart(std::pair<float, float> pt, float k = 1) {
	float phi = pt.first;
	float dist = pt.second;
	int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + ORIGIN_X;
	int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + ORIGIN_Y;
	return std::make_pair(x, y);
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

void find_shape(Cloud& cloud, float q = 50, float scale = 0.09) {
	cloud.shape = {{cyl_to_cart(cloud.pts[0], scale)}};
	for (size_t i = 1; i < cloud.size; i++) {
		float dist0 = cloud.pts[i - 1].second;
		float dist1 = cloud.pts[i].second;

		if (std::fabs(dist0 - dist1) <= q) {
			cloud.shape.back().push_back(cyl_to_cart(cloud.pts[i], scale));
		}
		else {
			cloud.shape.push_back({cyl_to_cart(cloud.pts[i], scale)});
		}
	}
}

void load_cloud(const std::string & filename, Cloud & cloud, int k, float scale) {
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
	for (auto & pt : cloud.pts) {
		cloud.std += (cloud.avg - pt.second) * (cloud.avg - pt.second);
	}
	cloud.std = std::sqrt(cloud.std);
	find_shape(cloud, k, scale);
}

void rotate_cloud(Cloud& cloud, float angle) {
	for (auto& i : cloud.pts) {
		i.first += angle;
		if (i.first >= 360) i.first -= 360;
	}
}

void draw_pixel(uint8_t * mat, unsigned x, unsigned y, sf::Color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;

	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

sf::Color calc_color(int x, int y) {
	x = (float(x) / WIDTH) * 255;
	y = (float(y) / HEIGHT) * 255;

	uint8_t r = 255 - std::abs(int(x) - int(y));
	uint8_t g = (y / 2 + 128) * (float(255 - x) / 255);
	uint8_t b = (x + y > 255) ? (255) : (x + y);
	return sf::Color(r, g, b, 255);
}

void draw_point(uint8_t* mat, unsigned x, unsigned y, sf::Color c) {
	c = calc_color(x, y);
	draw_pixel(mat, x + 1, y, c);
	draw_pixel(mat, x + 1, y + 1, c);
	draw_pixel(mat, x, y, c);
	draw_pixel(mat, x, y + 1, c);
}

void draw_cloud_bars(uint8_t * mat, const Cloud & cloud) {
	unsigned max_height = 200;
	for (int j = 0; j < WIDTH; j++) {
		float dist = cloud.pts[size_t(j * cloud.size / WIDTH)].second;

		unsigned height = unsigned(std::round(dist / cloud.max * max_height));

		for (int i = 0; i < height; i++) {
			draw_pixel(mat, j, HEIGHT - i - 1, sf::Color(255, 255, 255));
		}
	}

	//for (int x = 0; x < WIDTH; x++) {
	//	draw_pixel(mat, x, HEIGHT - max_height - 1, sf::Color(0, 0, 255));
	//}
}

void draw_line(uint8_t* mat, float x0, float y0, float x1, float y1, const sf::Color color) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		draw_point(mat, x0, y0, color);
		x0 += x; y0 += y;
	}
}

void draw_ray(uint8_t* mat, float x0, float y0, float x1, float y1, const sf::Color color) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	while (x0 < WIDTH && x0 >= 0 && y0 < HEIGHT && y0 >= 0) {
		draw_point(mat, x0, y0, color);
		x0 += x; y0 += y;
	}
}

void draw_cloud(uint8_t* mat, const Cloud& cloud, float k = 0.04, sf::Color c = sf::Color::White) {
	for (auto& pt : cloud.pts) {
		float phi = pt.first;
		float dist = pt.second;
		int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + ORIGIN_X;
		int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + ORIGIN_Y;

		draw_point(mat, x, y, c);
	}
}

void draw_cloud_shape(uint8_t* mat, const Cloud& cloud, float k = 0.04, sf::Color c = sf::Color::White) {
	std::pair<int, int> pt_prev, pt = cloud.pts.back();
	float dist_prev, dist = cloud.pts.back().second;
	for (int i = 0; i < cloud.shape.size(); i++) {
		pt_prev = pt;
		dist_prev = dist;
		pt = cloud.shape[i].front();
		draw_point(mat, pt.first, pt.second, c);
		for (int j = 1; j < cloud.shape[i].size(); j++) {
			pt_prev = pt;
			pt = cloud.shape[i][j];
			draw_line(mat, pt_prev.first, pt_prev.second, pt.first, pt.second, c);
		}
	}
}

void draw_background(uint8_t* mat, sf::Color c) {
	for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i += CHANNELS) {
		mat[i + 0] = c.r;
		mat[i + 1] = c.g;
		mat[i + 2] = c.b;
		mat[i + 3] = c.a;
	}
}

bool save_screenshot(uint8_t* mat) {
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	texture.update(mat);
	return texture.copyToImage().saveToFile(std::to_string(time(0)) + ".png");
}

int main(int argc, char** argv) {
	Cloud cloud;
	load_cloud("../clouds/10", cloud, 50000, 0.08);
	// save_cloud_cart("../10.txt", cloud);

	uint8_t* mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	draw_background(mat, sf::Color::Black);

	window_.create(sf::VideoMode(WIDTH, HEIGHT), TITLE, STYLE);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);

	uint8_t color = 255;
	float k = 1, epsilon = 0;
	while (running_) {
		sf::Event event;
		while (window_.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running_ = false;
				break;
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Up) {
					k += 30;
				}
				else if (event.key.code == sf::Keyboard::Down) {
					k -= 30;
					if (k <= 0) k = 1;
				}
				else if (event.key.code == sf::Keyboard::W) {
					epsilon += 0.1;
				}
				else if (event.key.code == sf::Keyboard::S) {
					epsilon -= 0.1;
					if (epsilon <= 0) epsilon = 0;
				}
				else if (event.key.code == sf::Keyboard::K) {
					save_screenshot(mat);
				}
				find_shape(cloud, k, 0.08);
				smooth_shape(cloud, epsilon);
				break;
			}
		}
		
		draw_background(mat, sf::Color(0, 0, 0));
		draw_cloud_bars(mat, cloud);
		draw_cloud_shape(mat, cloud, 0.08, sf::Color(color / 2, color / 2, color / 2));
		//draw_cloud(mat, cloud, 0.08, sf::Color(color, color, color));
		draw_pixel(mat, ORIGIN_X, ORIGIN_Y, sf::Color(255, 0, 0));

		window_.draw(sprite);
		texture.update(mat);
		window_.display();
	}

	delete[] mat;
	return 0;
}