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
const int ORIGIN_X = WIDTH / 2;
const int ORIGIN_Y = HEIGHT / 2 - HEIGHT / 3;
const std::string TITLE = "STM32F746G-DISCO | 480x272";
const sf::Uint32 STYLE = sf::Style::Close | sf::Style::Titlebar;
sf::RenderWindow window_;
bool running_ = true;

struct Cloud {
	std::vector<std::pair<float, float>> pts;
	std::vector<std::vector<size_t>> shape;
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

void find_shape(Cloud& cloud, float q = 50) {
	cloud.shape = { {0} };
	for (size_t i = 1; i < cloud.size; i++) {
		float dist0 = cloud.pts[i - 1].second;
		float dist1 = cloud.pts[i].second;

		if (std::fabs(dist0 - dist1) <= q) {
			cloud.shape.back().push_back(i);
		}
		else {
			cloud.shape.push_back({ i });
		}
	}
}

void load_cloud(const std::string & filename, Cloud & cloud, int q = 50) {
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
	find_shape(cloud, q);
}

void draw_pixel(uint8_t * mat, unsigned x, unsigned y, sf::Color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;

	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

void draw_point(uint8_t* mat, unsigned x, unsigned y, sf::Color c) {
	draw_pixel(mat, x + 1, y + 1, c);
	draw_pixel(mat, x + 1, y, c);
	draw_pixel(mat, x, y + 1, c);
	draw_pixel(mat, x, y, c);
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

	for (int x = 0; x < WIDTH; x++) {
		draw_pixel(mat, x, HEIGHT - max_height - 1, sf::Color(0, 0, 255));
	}
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
	std::pair<int, int> pt_prev, pt = cyl_to_cart(cloud.pts.back(), k);
	float dist_prev, dist = cloud.pts.back().second;
	for (int i = 0; i < cloud.shape.size(); i++) {
		pt_prev = pt;
		dist_prev = dist;
		pt = cyl_to_cart(cloud.pts[cloud.shape[i].front()], k);
		dist = cloud.pts[cloud.shape[i].front()].second;
		// draw_ray(mat, pt.first, pt.second, pt_prev.first, pt_prev.second, sf::Color::Magenta);
		//if (i > 0) {
		//	if (dist_prev < dist) {
		//		float x1 = float(pt_prev.first + (pt_prev.first - ORIGIN_X) / 10);
		//		float x2 = float(pt_prev.second + (pt_prev.second - ORIGIN_Y) / 10);
		//		draw_ray(mat, pt_prev.first, pt_prev.second, x1, x2, sf::Color(64, 64, 64));
		//	}
		//	else {
		//		float x1 = float(pt.first + (pt.first - ORIGIN_X) / 10);
		//		float x2 = float(pt.second + (pt.second - ORIGIN_Y) / 10);
		//		draw_ray(mat, pt.first, pt.second, x1, x2, sf::Color(64, 64, 64));
		//	}
		//}
		draw_pixel(mat, pt.first, pt.second, c);
		for (int j = 1; j < cloud.shape[i].size(); j++) {
			pt_prev = pt;
			pt = cyl_to_cart(cloud.pts[cloud.shape[i][j]], k);
			draw_line(mat, pt_prev.first, pt_prev.second, pt.first, pt.second, sf::Color(128, 128, 128));
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

///

void add_to_pixel(uint8_t* mat, unsigned x, unsigned y, sf::Color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;
	auto r = (WIDTH * y + x) * CHANNELS + 0;
	auto g = (WIDTH * y + x)* CHANNELS + 1;
	auto b = (WIDTH * y + x)* CHANNELS + 2;
	auto a = (WIDTH * y + x)* CHANNELS + 3;


	mat[r] = (int(mat[r]) + c.r) > 255 ? 255 : (mat[r] + c.r);
	mat[g] = (int(mat[g]) + c.g) > 255 ? 255 : (mat[g] + c.g);
	mat[b] = (int(mat[b]) + c.b) > 255 ? 255 : (mat[b] + c.b);
	mat[a] = (int(mat[a]) + c.a) > 255 ? 255 : (mat[a] + c.a);
}

void cloud_to_mat(std::vector<std::pair<float, float>>& cloud, uint8_t* mat, sf::Color c, float k=0.04) {
	for (auto& pt : cloud) {
		float phi = pt.first;
		float dist = pt.second;
		int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + WIDTH / 2;
		int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + HEIGHT / 2;

		add_to_pixel(mat, x, y, c);
	}
}

void rotate_cloud(std::vector<std::pair<float, float>>& cloud, float angle) {
	for (auto& pt : cloud) {
		pt.first += angle;
		if (pt.first >= 360) pt.first -= 360;
	}
}

void save_cloud_cart(const std::string& filename, const std::vector<std::pair<float, float>>& cloud) {
	std::ofstream file(filename);
	for (auto& pt : cloud) {
		float phi = pt.first;
		float dist = pt.second;
		float x = dist * std::sin(phi * (acos(-1) / 180.0));
		float y = dist * std::cos(phi * (acos(-1) / 180.0));

		for (int i = 0; i < 20; i++)
			file << x << " ; " << y << " ; " << i << "0.0" << std::endl;
	}
}

int main(int argc, char** argv) {
	Cloud cloud;
	load_cloud("../clouds/10", cloud, 50);
	// save_cloud_cart("../10.txt", cloud);

	uint8_t* mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	draw_background(mat, sf::Color::Black);

	window_.create(sf::VideoMode(WIDTH, HEIGHT), TITLE, STYLE);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);

	float k = 50;

	while (running_) {
		sf::Event event;
		while (window_.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running_ = false;
				break;
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::W) {
					k += 30;
				}
				else if (event.key.code == sf::Keyboard::S) {
					if (k > 1)
						k -= 30;
				}
				find_shape(cloud, k);
			}
		}
		
		draw_background(mat, sf::Color(0, 0, 0));
		window_.draw(sprite);
		draw_cloud_shape(mat, cloud, 0.09, sf::Color(128, 128, 128));
		draw_cloud(mat, cloud, 0.09);
		draw_pixel(mat, ORIGIN_X, ORIGIN_Y, sf::Color(255, 0, 0));
		texture.update(mat);

		//for (auto& w : v)
		//	window_.draw(w);

		window_.display();
	}

	delete[] mat;
	return 0;
}