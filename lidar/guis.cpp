#include "guis.h"

TerminalGUI::TerminalGUI() {

}

bool TerminalGUI::update(const Cloud& cloud) {
	return true;
}


SFMLGUI::SFMLGUI(const SFMLGUISettings& settings) {
	sets_ = settings;
	
	sf::ContextSettings window_settings;
	window_settings.antialiasingLevel = sets_.antialiasing;

	window_.create(
		sf::VideoMode(sets_.width, sets_.height),
		"Lidar", 
		sf::Style::Close | sf::Style::Titlebar,
		window_settings
	);
}

bool SFMLGUI::update(const Cloud& cloud) {
	if (!sets_.running) {
		window_.close();
		return false;
	}

	handle_input();

	if (sets_.scale == 0)
		sets_.scale = calc_scale(cloud);

	window_.clear(sets_.color_background);
	if (sets_.render_grid) {
		render_grid();
	}

	if (sets_.pts_display_mode == SFMLGUISettings::DOTS_LINES) {
		render_connected_cloud(cloud);
		render_cloud(cloud);
	}
	else if (sets_.pts_display_mode == SFMLGUISettings::LINES) {
		render_connected_cloud(cloud);
	}
	else if (sets_.pts_display_mode == SFMLGUISettings::DOTS) {
		render_cloud(cloud);
	}
	render_point(sets_.origin_x, sets_.origin_y, Color::Red);

	window_.display();
	sf::sleep(sf::milliseconds(sets_.sleep_time_ms));
	return true;
}

void SFMLGUI::handle_input() {
	sf::Event event;
	while (window_.pollEvent(event)) {
		if (event.type == sf::Event::Resized) {
			sets_.width = event.size.width;
			sets_.width = event.size.height;
			sets_.origin_x = event.size.width / 2;
			sets_.origin_y = event.size.height / 2;
		}
		if (event.type == sf::Event::Closed)
			sets_.running = false;
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::S);
				// save_screenshot();
			if (event.key.code == sf::Keyboard::T);
				// save_txt();
			if (event.key.code == sf::Keyboard::Up)
				sets_.scale += (event.key.shift ? 0.1 : 0.01);
			if (event.key.code == sf::Keyboard::Down)
				sets_.scale = std::max(0.0, sets_.scale - (event.key.shift ? 0.1 : 0.01));
			if (event.key.code == sf::Keyboard::R)
				sets_.render_mouse_ray = !sets_.render_mouse_ray;
		}

			//// Screenshot saving event
			//if (event.key.code == sf::Keyboard::S) {
			//	if (save_screenshot(mat, output_dir)) std::cout << "Screenshot saved." << std::endl;
			//	else std::cerr << "ERROR: Something went wrong while saving screenshot." << std::endl;
			//}
			//// TXT cloud saving event
			//if (event.key.code == sf::Keyboard::T) {
			//	if (save_txt(cloud, output_dir)) std::cout << "TXT cloud saved." << std::endl;
			//	else std::cerr << "ERROR: Something went wrong while saving TXT cloud." << std::endl;
			//}
			//// Stop/Start point cloud rotating
			//if (event.key.code == sf::Keyboard::P) {
			//	rotate = !rotate;
			//}
			//// Cloud rotation event
			//if (event.key.code == sf::Keyboard::Right) {
			//	float rotation = 5.0f;
			//	if (event.key.control) rotation = 1.0f;
			//	if (event.key.shift) rotation = 30.0f;
			//	rotate_cloud(cloud, rotation);
			//}
			//// Cloud rotation event
			//if (event.key.code == sf::Keyboard::Left) {
			//	float rotation = 5.0f;
			//	if (event.key.control) rotation = 1.0f;
			//	if (event.key.shift) rotation = 30.0f;
			//	rotate_cloud(cloud, -rotation);
			//}
			//// Cloud scale event
			//if (event.key.code == sf::Keyboard::Down) {
			//	scale -= 0.01f;
			//	if (event.key.control) scale -= 0.001f;
			//	if (event.key.shift) scale -= 0.05f;

			//	if (scale <= 0) scale = 0.001;
			//}
			//// Cloud scale event
			//if (event.key.code == sf::Keyboard::Up) {
			//	scale += 0.01f;
			//	if (event.key.control) scale += 0.001f;
			//	if (event.key.shift) scale += 0.05f;

			//	if (scale <= 0) scale = 0.001;
			//}
			//// Mouse ray event
			//if (event.key.code == sf::Keyboard::R) {
			//	mouse_ray = !mouse_ray;
			//}
			//// Point cloud display mode event
			//if (event.key.code == sf::Keyboard::M) {
			//	point_cloud_display_mode = (point_cloud_display_mode + 1) % 3;
			//}
			//// Point cloud display mode event
			//if (event.key.code == sf::Keyboard::C) {
			//	coloring = (coloring + 1) % 2;
			//}
	}
}

void SFMLGUI::render_grid() {

}

void SFMLGUI::render_cloud_bars(const Cloud& cloud) {

}

void SFMLGUI::render_cloud(const Cloud& cloud, float lightness) {
	if (cloud.size == 0)
		return;

	int cnt = 1;
	for (int i = 0; i < cloud.size; i++) {
		Color color;
		if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
			color = calc_color_from_angle(float(cnt) / float(cloud.size), lightness);
		}
		else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
			color = calc_color_from_dist(cloud.pts_cyl[i].dist, cloud.max, lightness);
		}
		render_point(cloud.pts_cart[i].x, cloud.pts_cart[i].y, color);
		cnt++;
	}
}

void SFMLGUI::render_connected_cloud(const Cloud& cloud) {

}

void SFMLGUI::render_point(int x, int y, const Color& color) {
	auto pt = sf::CircleShape(2);
	pt.setOrigin(1, 1);
	pt.setPosition(x, y);
	pt.setFillColor(color);
	window_.draw(pt);
}

void SFMLGUI::render_mark() {

}

bool SFMLGUI::save_screenshot() {
	return false;
}

float SFMLGUI::calc_scale(const Cloud& cloud) {
	return float(sets_.height) * 0.7f / cloud.max;
}

Color SFMLGUI::calc_color_from_angle(float v, float lightness) {
	Color c0, c1;
	if (v >= 0 && v <= 0.33f) {
		c0 = Color(0, 255, 255);
		c1 = Color(255, 0, 255);
	}
	else if (v <= 0.66f) {
		v -= 0.33f;
		c0 = Color(255, 255, 0);
		c1 = Color(0, 255, 255);
	}
	else if (v <= 1.0f) {
		v -= 0.66f;
		c0 = Color(255, 0, 255);
		c1 = Color(255, 255, 0);
	}
	else {
		return Color(255, 255, 255);
	}

	c0.r *= float(v) / 0.34f;
	c0.g *= float(v) / 0.34f;
	c0.b *= float(v) / 0.34f;
	c1.r *= 1.0f - float(v) / 0.34f;
	c1.g *= 1.0f - float(v) / 0.34f;
	c1.b *= 1.0f - float(v) / 0.34f;
	return Color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness, float(c0.b + c1.b) * lightness);
}

Color SFMLGUI::calc_color_from_dist(float dist, float max, float lightness) {
	float v = dist / max;
	Color c0, c1;
	if (v >= 0 && v <= 0.33f) {
		c0 = Color(255, 255, 0);
		c1 = Color(255, 0, 0);
	}
	else if (v <= 0.66f) {
		v -= 0.33f;
		c0 = Color(0, 255, 0);
		c1 = Color(255, 255, 0);
	}
	else if (v <= 1.0f) {
		v -= 0.66f;
		c0 = Color(0, 0, 255);
		c1 = Color(0, 255, 0);
	}
	else {
		return Color(255, 255, 255);
	}

	c0.r *= float(v) / 0.34f;
	c0.g *= float(v) / 0.34f;
	c0.b *= float(v) / 0.34f;
	c1.r *= 1.0f - float(v) / 0.34f;
	c1.g *= 1.0f - float(v) / 0.34f;
	c1.b *= 1.0f - float(v) / 0.34f;
	return Color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness, float(c0.b + c1.b) * lightness);
}