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

const unsigned WIDTH = 480;
const unsigned HEIGHT = 272;
const unsigned CHANNELS = 4;
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

void find_shape(Cloud& cloud, float k = 50, float q = 5) {
	cloud.shape = { {0} };
	for (size_t i = 1; i < cloud.size; i += 1) {
		float dist0 = cloud.pts[i - 1].second;
		float dist1 = cloud.pts[i].second;

		if (std::fabs(dist0 - dist1) <= q) {
			cloud.shape.back().back() = i;
		}
		else if (std::fabs(dist0 - dist1) <= k) {
			cloud.shape.back().push_back(i);
		}
		else {
			cloud.shape.push_back({ i });
		}
	}
}

void load_cloud(const std::string & filename, Cloud & cloud) {
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
}

void draw_pixel(uint8_t * mat, unsigned x, unsigned y, sf::Color c) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1)
		return;

	mat[(WIDTH * y + x) * CHANNELS + 0] = c.r;
	mat[(WIDTH * y + x) * CHANNELS + 1] = c.g;
	mat[(WIDTH * y + x) * CHANNELS + 2] = c.b;
	mat[(WIDTH * y + x) * CHANNELS + 3] = c.a;
}

void draw_cloud_bars(uint8_t* mat, const Cloud & cloud) {
	unsigned max_height = 200;
	for (int j = 0; j < WIDTH; j++) {
		float dist = cloud.pts[size_t(j * cloud.size / WIDTH)].second;

		unsigned height = std::round(dist / cloud.max * max_height);

		for (int i = 0; i < height; i++) {
			draw_pixel(mat, j, HEIGHT - i - 1, sf::Color(255, 255, 255));
		}
	}

	for (int x = 0; x < WIDTH; x++) {
		draw_pixel(mat, x, HEIGHT - max_height - 1, sf::Color(0, 0, 255));
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
	Cloud cloud;
	load_cloud("../clouds/10", cloud);
	// save_cloud_cart("../10.txt", cloud);

	uint8_t* mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	fill_mat(mat, sf::Color::Black);

	window_.create(sf::VideoMode(WIDTH, HEIGHT), TITLE, STYLE);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);

	float k = 50, q = 5;

	while (running_) {
		sf::Event event;
		while (window_.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running_ = false;
				break;
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::W) {
					k += 100;
				}
				else if (event.key.code == sf::Keyboard::S) {
					k -= 100;
				}
				else if (event.key.code == sf::Keyboard::A) {
					q += 100;
				}
				else if (event.key.code == sf::Keyboard::D) {
					q -= 100;
				}
			}
		}

		find_shape(cloud, k, q);
		std::vector<sf::VertexArray> v(cloud.shape.size(), sf::VertexArray(sf::PrimitiveType::LinesStrip));
		for (int i = 0; i < cloud.shape.size(); i++) {
			for (int j = 0; j < cloud.shape[i].size(); j++) {
				float phi = cloud.pts[cloud.shape[i][j]].first;
				float dist = cloud.pts[cloud.shape[i][j]].second;
				int x = std::round(dist * std::sin(phi * (acos(-1) / 180.0)) * 0.04) + WIDTH / 2;
				int y = std::round(dist * std::cos(phi * (acos(-1) / 180.0)) * 0.04) + HEIGHT / 2;

				v[i].append(sf::Vertex(sf::Vector2f(x, y)));
			}
		}

		texture.update(mat);

		window_.draw(sprite);
		for (auto& w : v)
			window_.draw(w);
		window_.display();
	}

	delete[] mat;
	return 0;
}