#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <utility>
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <SFML\Graphics.hpp>

/// WINDOW
const unsigned WIDTH = 480;
const unsigned HEIGHT = 272;
const unsigned CHANNELS = 4;
const std::string TITLE = "STM32F746G-DISCO | 480x272";
const sf::Uint32 STYLE = sf::Style::Close | sf::Style::Titlebar;
sf::RenderWindow window_;
bool running_ = true;

size_t load_cloud(const std::string & filename, std::vector<std::pair<float, float>>& cloud) {
	std::ifstream file(filename);
	size_t cnt = 0;
	while (file) {
		std::string line;
		std::getline(file, line);
		if (line.empty() || line[0] == '#')
			continue;
		float angle, dist;
		std::stringstream sline(line);
		sline >> angle >> dist;
		cloud.push_back(std::make_pair(angle, dist));
		cnt++;
	}
	return cnt;
}

void draw_pixel(uint8_t* mat, unsigned x, unsigned y, sf::Color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;

	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

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

void fill_mat(uint8_t* mat, sf::Color c) {
	for (int i = 0; i < WIDTH * HEIGHT * CHANNELS; i += CHANNELS) {
		mat[i + 0] = c.r;
		mat[i + 1] = c.g;
		mat[i + 2] = c.b;
		mat[i + 3] = c.a;
	}
}

void cloud_to_mat(std::vector<std::pair<float, float>>& cloud, uint8_t* mat, sf::Color c, float k=0.04) {
	for (auto& pt : cloud) {
		float phi = pt.first;
		float dist = pt.second;
		int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * k) + WIDTH / 2;
		int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * k) + HEIGHT / 2;

		draw_pixel(mat, x - 1, y, c);
		draw_pixel(mat, x + 1, y, c);
		draw_pixel(mat, x, y - 1, c);
		draw_pixel(mat, x, y + 1, c);
		draw_pixel(mat, x, y, c);
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
	std::vector<std::pair<float, float>> cloud;
	load_cloud("../clouds/10", cloud);
	save_cloud_cart("../10.txt", cloud);

	uint8_t* mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	fill_mat(mat, sf::Color::Black);

	window_.create(sf::VideoMode(WIDTH, HEIGHT), TITLE, STYLE);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	while (running_) {
		sf::Event event;
		while (window_.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running_ = false;
				break;
			}
		}
		rotate_cloud(cloud, 0.1);
		fill_mat(mat, sf::Color::Black);
		cloud_to_mat(cloud, mat, sf::Color(255, 255, 255), 0.0390);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 180, 255 - 180, 255 - 180), 0.0395);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 150, 255 - 150, 255 - 150), 0.0400);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 120, 255 - 120, 255 - 120), 0.0405);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 90 , 255 - 90 , 255 - 90 ), 0.0410);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 60 , 255 - 60 , 255 - 60 ), 0.0415);
		//cloud_to_mat(cloud, mat, sf::Color(255 - 30 , 255 - 30 , 255 - 30 ), 0.0420);
		//cloud_to_mat(cloud, mat, sf::Color(255, 255, 255), 0.0425);
		draw_pixel(mat, WIDTH / 2 - 1, HEIGHT / 2 - 1, sf::Color(255, 0, 0));
		draw_pixel(mat, WIDTH / 2 + 1, HEIGHT / 2 + 1, sf::Color(255, 0, 0));
		draw_pixel(mat, WIDTH / 2 + 1, HEIGHT / 2 - 1, sf::Color(255, 0, 0));
		draw_pixel(mat, WIDTH / 2 - 1, HEIGHT / 2 + 1, sf::Color(255, 0, 0));
		draw_pixel(mat, WIDTH / 2, HEIGHT / 2, sf::Color(255, 0, 0));
		texture.update(mat);

		window_.draw(sprite);
		window_.display();
	}

	delete[] mat;
	return 0;
}