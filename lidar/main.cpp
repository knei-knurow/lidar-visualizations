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

int main(int argc, char** argv) {
	Cloud cloud;
	load_cloud("../clouds/40", cloud, 0, 0);
	// save_cloud_cart("../10.txt", cloud);

	uint8_t* mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	draw_background(mat, color::Black);

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT),
		"STM32F746G-DISCO | 480x272",
		sf::Style::Close | sf::Style::Titlebar);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);

	uint8_t c = 255;
	float k = 0, epsilon = 0;
	int cnt = 0;
	bool running = true;
	while (running) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				running = false;
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
				// find_shape(cloud, k, 0.08);
				break;
			}
		}
		
		draw_background(mat, COLOR_BACKGROUND);
		draw_grid(mat, COLOR_GRID);
		draw_point(mat, ORIGIN_X, ORIGIN_Y, color(255, 255, 255), 1.0);
		//draw_cloud_shape(mat, cloud, 3, 0.4);
		//draw_cloud_shape(mat, cloud, 0, 0.6);
		//draw_cloud_shape(mat, cloud, -3, 0.8);
		//draw_cloud_shape(mat, cloud, -6, 1.0);
		draw_cloud_bars(mat, cloud);


		rotate_cloud(cloud, 0.1);
		draw_connected_cloud(mat, cloud, 0, +3, 0.4, false);
		draw_connected_cloud(mat, cloud, 0, +0, 0.6, false);
		draw_connected_cloud(mat, cloud, 0, -3, 0.8, false);
		draw_connected_cloud(mat, cloud, 0, -6, 1.0, true);
		/// find_shape(cloud, 10000);
		// smooth_shape(cloud, epsilon);

		/*for (int i = 0; i < WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				draw_pixel(mat, i, j, calc_color(float(i) / WIDTH));
			}
		}*/

		//save_screenshot(mat, std::to_string(cnt++) + ".png");
		window.draw(sprite);
		texture.update(mat);
		window.display();

		//if (cnt >= 360.0 / 5) break;
	}

	delete[] mat;
	return 0;
}