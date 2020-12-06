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
#include "app.h"
#include "communication.h"

int main(int argc, char** argv) {
	using namespace rp::standalone;
	bool running = true;
	bool rotate = false;
	bool mouse_ray = true;
	unsigned coloring = 0;
	unsigned point_cloud_display_mode = 2;
	Cloud cloud;
	rplidar::RPlidarDriver * lidar = nullptr;
	rplidar_response_measurement_node_hq_t * buffer = nullptr;
	uint8_t * mat = nullptr;
	
	// Print help
	if (check_arg_exist(argc, argv, "-h") || argc == 1) {
		print_help();
		running = false;
	}

	// Set input point cloud filename
	std::string file = get_arg(argc, argv, "-f");
	if (!file.empty()) {
		if (!load_cloud(file, cloud)) {
			running = false;
			file = "";
		}
		rotate = true;
	}

	// Set output directory
	std::string output_dir = get_arg(argc, argv, "-o");
	if (output_dir.empty()) {
		output_dir = ".";
	}

	// Set RPLIDAR port
	std::string port = get_arg(argc, argv, "-p");
	if (!port.empty() && file.empty()) {
		lidar = rplidar::RPlidarDriver::CreateDriver();
		if (!rplidar_launch(lidar, port)) {
			running = false;
			port = "";
		}
	}
	
	// Set scenario - general program behaviour
	std::string scenario = get_arg(argc, argv, "-S");
	if (scenario == "0") {
		// TODO
	}
	else if (!scenario.empty()) {
		std::cerr << "ERROR: Unknown scenario. Running with default settings." << std::endl;
	}

	// Set scale
	std::string scale_s = get_arg(argc, argv, "-s");
	float scale = 0.04;
	if (!scale_s.empty()) {
		try {
			scale = std::fabs(std::stod(scale_s));
		}
		catch (...) {
			std::cerr << "ERROR: Invalid scale." << std::endl;
		}
	}

	// Set mouse ray
	if (check_arg_exist(argc, argv, "-r")) {
		mouse_ray = false;
	}

	check_invalid_args(argc, argv);

	if ((file == "" && port == "") && running) {
		std::cerr << "ERROR: No valid port or input file specified." << std::endl;
		running = false;
	}

	// Main program loop
	buffer = new rplidar_response_measurement_node_hq_t[rplidar::RPlidarDriver::MAX_SCAN_NODES];
	mat = new uint8_t[WIDTH * HEIGHT * CHANNELS];
	sf::Texture texture;
	texture.create(WIDTH, HEIGHT);
	sf::Sprite sprite(texture);
	sf::RenderWindow window;
	if (running) {
		window.create(sf::VideoMode(WIDTH, HEIGHT), "Lidar", sf::Style::Close | sf::Style::Titlebar);
	}
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
					if (save_screenshot(mat, output_dir)) std::cout << "Screenshot saved." << std::endl;
					else std::cerr << "ERROR: Something went wrong while saving screenshot." << std::endl;
				}
				// TXT cloud saving event
				if (event.key.code == sf::Keyboard::T) {
					if (save_txt(cloud, output_dir)) std::cout << "TXT cloud saved." << std::endl;
					else std::cerr << "ERROR: Something went wrong while saving TXT cloud." << std::endl;
				}
				// Stop/Start point cloud rotating
				if (event.key.code == sf::Keyboard::P) {
					rotate = !rotate;
				}
				// Cloud rotation event
				if (event.key.code == sf::Keyboard::Right) {
					float rotation = 5.0f;
					if (event.key.control) rotation = 1.0f;
					if (event.key.shift) rotation = 30.0f;
					rotate_cloud(cloud, rotation);
				}
				// Cloud rotation event
				if (event.key.code == sf::Keyboard::Left) {
					float rotation = 5.0f;
					if (event.key.control) rotation = 1.0f;
					if (event.key.shift) rotation = 30.0f;
					rotate_cloud(cloud, -rotation);
				}
				// Cloud scale event
				if (event.key.code == sf::Keyboard::Down) {
					scale -= 0.01f;
					if (event.key.control) scale -= 0.001f;
					if (event.key.shift) scale -= 0.05f;
					
					if (scale <= 0) scale = 0.001;
				}
				// Cloud scale event
				if (event.key.code == sf::Keyboard::Up) {
					scale += 0.01f;
					if (event.key.control) scale += 0.001f;
					if (event.key.shift) scale += 0.05f;
					
					if (scale <= 0) scale = 0.001;
				}
				// Mouse ray event
				if (event.key.code == sf::Keyboard::R) {
					mouse_ray = !mouse_ray;
				}
				// Point cloud display mode event
				if (event.key.code == sf::Keyboard::M) {
					point_cloud_display_mode = (point_cloud_display_mode + 1) % 3;
				}
				// Point cloud display mode event
				if (event.key.code == sf::Keyboard::C) {
					coloring = (coloring + 1) % 2;
				}
			}
		}

		if (lidar) {
			size_t count;
			if (!rplidar_scan(lidar, buffer, count, false)) break;
			load_cloud_from_buffer(buffer, count, cloud);
		}

		if (rotate) {
			rotate_cloud(cloud, 0.5);
		}

		draw_background(mat, COLOR_BACKGROUND);
		draw_grid(mat, COLOR_GRID);

		auto mouse = sf::Mouse::getPosition(window);
		auto pt_i = mouse_ray_to_point(cloud, mouse.x, mouse.y);
		std::pair<int, int> pt;
		if (mouse_ray) {
			if (pt_i != size_t(-1)) {
				float ray_scale = scale == 0 ? calc_scale(cloud) : scale;
				pt = cyl_to_cart(cloud.pts[pt_i], ray_scale);
				draw_line(mat, ORIGIN_X, ORIGIN_Y, pt.first, pt.second, COLOR_BACKGROUND_GRID);
			}
		}

		draw_cloud_bars(mat, cloud, coloring);

		if (point_cloud_display_mode == 2) {
			draw_connected_cloud(mat, cloud, scale, 0, 0.5, false, coloring);
			draw_cloud(mat, cloud, scale, 0, 1, true, coloring);
		}
		else if (point_cloud_display_mode == 1) {
			draw_connected_cloud(mat, cloud, scale, 0, 1, true, coloring);
		}
		else if (point_cloud_display_mode == 0) {
			draw_cloud(mat, cloud, scale, 0, 1, true, coloring);
		}

		draw_point(mat, ORIGIN_X, ORIGIN_Y, color(255, 0, 0), 1.0);

		if (pt_i != size_t(-1) && mouse_ray) {
			draw_mark(mat, pt.first, pt.second, cloud.pts[pt_i].second);
		}

		window.draw(sprite);
		texture.update(mat);
		window.display();
	}

	// Cleaning up
	if (lidar) {
		rplidar_stop(lidar);
	}
	delete[] mat;
	delete[] buffer;
	return 0;
}