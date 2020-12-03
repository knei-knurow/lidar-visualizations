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
#include "communication.h"

int main(int argc, char** argv) {
	using namespace rp::standalone;
	bool running = true;
	bool rotate = false;
	Cloud cloud;
	rplidar::RPlidarDriver * lidar = nullptr;
	rplidar_response_measurement_node_hq_t * buffer = nullptr;
	uint8_t * mat = nullptr;
	
	if (argc <= 1) {
		lidar = rplidar::RPlidarDriver::CreateDriver();
		if (!rplidar_launch(lidar)) {
			running = false;
		}
	}
	else if (std::strcmp(argv[1], "--help") == 0 || std::strcmp(argv[1], "-h") == 0) {
		std::cout << "-----------------------------------------------------------" << std::endl;
		std::cout << "Lidar Visualizations" << std::endl;
		std::cout << "-----------------------------------------------------------" << std::endl;
		std::cout << "Authors: Bartek Dudek, Szymon Bednorz" << std::endl;
		std::cout << "Source: https://github.com/knei-knurow/lidar-visualizations" << std::endl;
		std::cout << std::endl;
		std::cout << "Usage:" << std::endl;
		std::cout << "\tlidar [source type] [source]" << std::endl;
		std::cout << std::endl;
		std::cout << "Source Types:" << std::endl;
		std::cout << "\tfile\tfile with lines containing angle [deg] and distance [mm] separated by whitespaces" << std::endl;
		std::cout << "\tport\tRPLidar port" << std::endl;
		std::cout << std::endl;
		std::cout << "GUI Mode Keyboard Shortcuts:" << std::endl;
		std::cout << "\tT\tsave point cloud as TXT" << std::endl;
		std::cout << "\tS\tsave screenshot" << std::endl;
		std::cout << "\tA/D\trotate cloud (faster with shift, slower with ctrl)" << std::endl;
		std::cout << "\tP\trotation on/off" << std::endl;
		running = false;
	}
	else if (argc == 3) {
		if (std::strcmp(argv[1], "file") == 0) {
			load_cloud(argv[2], cloud, 0, 0);
			rotate = true;
			if (cloud.size == 0) {
				std::cerr << "Error: File does not contain a valid cloud." << std::endl;
				running = false;
			}
		}
		else if (std::strcmp(argv[1], "port") == 0) {
			lidar = rplidar::RPlidarDriver::CreateDriver();
			if (!rplidar_launch(lidar, argv[2])) {
				running = false;
			}
		}
		else {
			std::cerr << "Error: Unknown source type." << std::endl;
			running = false;
		}
	}
	else {
		std::cerr << "Error: Invalid parameters passed. Run `lidar.exe -h` for help." << std::endl;
		running = false;
	}

	if (!running) {
		return 0;
	}

	buffer = new rplidar_response_measurement_node_hq_t[rplidar::RPlidarDriver::MAX_SCAN_NODES];
	mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Lidar", sf::Style::Close | sf::Style::Titlebar);
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	while (running) {
		sf::sleep(sf::milliseconds(1000 / 30));
		sf::Event event;
		while (window.pollEvent(event)) {
			// Exit window event
			if (event.type == sf::Event::Closed) {
				running = false;
				break;
			}
			else if (event.type == sf::Event::KeyPressed) {
				// Screenshot saving event
				if (event.key.code == sf::Keyboard::S) {
					if (save_screenshot(mat)) std::cout << "Screenshot saved." << std::endl;
					else std::cerr << "ERROR: Something went wrong while saving screenshot." << std::endl;
				}
				// TXT cloud saving event
				if (event.key.code == sf::Keyboard::T) {
					if (save_txt(cloud)) std::cout << "TXT cloud saved." << std::endl;
					else std::cerr << "ERROR: Something went wrong while saving TXT cloud." << std::endl;
				}
				// Stop/Start point cloud rotating
				if (event.key.code == sf::Keyboard::P) {
					rotate = !rotate;
				}
				// Cloud rotation event
				if (event.key.code == sf::Keyboard::D) {
					float rotation = 5.0f;
					if (event.key.control) rotation = 1.0f;
					if (event.key.shift) rotation = 30.0f;
					rotate_cloud(cloud, rotation);
				}
				// Cloud rotation event
				if (event.key.code == sf::Keyboard::A) {
					float rotation = 5.0f;
					if (event.key.control) rotation = 1.0f;
					if (event.key.shift) rotation = 30.0f;
					rotate_cloud(cloud, -rotation);
				}
			}
		}


		if (lidar) {
			size_t count;
			if (!rplidar_scan(lidar, buffer, count, false)) break;
			load_cloud_from_buffer(buffer, count, cloud);
		}

		draw_background(mat, COLOR_BACKGROUND);
		draw_grid(mat, COLOR_GRID);
		draw_point(mat, ORIGIN_X, ORIGIN_Y, color(255, 0, 0), 1.0);
		draw_cloud_bars(mat, cloud);

		if (rotate) rotate_cloud(cloud, 0.5);
		draw_connected_cloud(mat, cloud, 0.04, +3, 0.4, false);
		draw_connected_cloud(mat, cloud, 0.04, +0, 0.6, false);
		draw_connected_cloud(mat, cloud, 0.04, -3, 0.8, false);
		draw_connected_cloud(mat, cloud, 0.04, -6, 1.0, true);

		window.draw(sprite);
		texture.update(mat);
		window.display();
	}

	if (lidar) {
		rplidar_stop(lidar);
	}
	delete[] mat;
	delete[] buffer;
	return 0;
}