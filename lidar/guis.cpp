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
		sets_.scale = calc_scale(cloud.max);

	window_.clear(sets_.color_background);
	if (sets_.render_grid) {
		render_grid();
	}

	if (sets_.render_grid) {
		render_cloud_bars(cloud);
	}

	if (sets_.pts_display_mode == SFMLGUISettings::DOTS_LINES) {
		render_connected_cloud(cloud);
	}
	else if (sets_.pts_display_mode == SFMLGUISettings::DOTS) {
		render_cloud(cloud);
	}
	else if (sets_.pts_display_mode == SFMLGUISettings::LINES) {
		render_connected_cloud(cloud, 1.0, false);
	}
	render_point(0, 0, Color::Red);

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
				sets_.origin_y -= 5;
			if (event.key.code == sf::Keyboard::Down)
				sets_.origin_y += 5;
			if (event.key.code == sf::Keyboard::Left)
				sets_.origin_x -= 5;
			if (event.key.code == sf::Keyboard::Right)
				sets_.origin_x += 5;
			if (event.key.code == sf::Keyboard::R)
				sets_.render_mouse_ray = !sets_.render_mouse_ray;
			if (event.key.code == sf::Keyboard::C)
				sets_.colormap = SFMLGUISettings::Colormap(
					(sets_.colormap + 1) % SFMLGUISettings::COLORMAP_COUNT
				);
			if (event.key.code == sf::Keyboard::M)
				sets_.pts_display_mode = SFMLGUISettings::PtsDispayMode(
					(sets_.pts_display_mode + 1) % SFMLGUISettings::PTS_DISPLAY_MODE_COUNT
				);
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
	for (int i = 0; i < 30; i++) {
		auto circle = sf::CircleShape(1000 * i * sets_.scale, 64);
		circle.setOrigin(circle.getRadius() - 1, circle.getRadius() - 1);
		circle.setPosition(sets_.origin_x, sets_.origin_y);
		circle.setFillColor(Color::Transparent);
		circle.setOutlineColor(sets_.color_grid);
		circle.setOutlineThickness(1);
		window_.draw(circle);
	}
}

void SFMLGUI::render_cloud_bars(const Cloud& cloud) {
	if (cloud.size == 0) return;

	unsigned max_width = 80;
	for (int j = 0; j < sets_.height; j++) {
		float dist = cloud.pts_cyl[size_t(j * cloud.size / sets_.height)].dist;

		int width = int(std::round(dist / cloud.max * max_width));

		Color color;
		if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
			color = calc_color_from_angle(float(j * cloud.size / sets_.height) / float(cloud.size));
		}
		else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
			color = calc_color_from_dist(cloud.pts_cyl[size_t(j * cloud.size / sets_.height)].dist, cloud.max, 1.0);
		}

		auto rect = sf::RectangleShape(sf::Vector2f(width, 1));
		rect.setPosition(0, j);
		rect.setFillColor(color);
		window_.draw(rect);
	}
}

void SFMLGUI::render_cloud(const Cloud& cloud, float lightness) {
	if (cloud.size == 0)
		return;

	for (int i = 0; i < cloud.size; i++) {
		Color color;
		if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
			color = calc_color_from_angle(float(i) / float(cloud.size), lightness);
		}
		else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
			color = calc_color_from_dist(cloud.pts_cyl[i].dist, cloud.max, lightness);
		}
		render_point(cloud.pts_cart[i].x, cloud.pts_cart[i].y, color);
	}
}

void SFMLGUI::render_connected_cloud(const Cloud& cloud, float lightness, bool render_points) {
	if (cloud.size == 0)
		return;

	sf::VertexArray vertex_arr(sf::PrimitiveType::LinesStrip);
	for (int i = 0; i < cloud.size; i++) {
		Color color;
		if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
			color = calc_color_from_angle(float(i) / float(cloud.size), lightness);
		}
		else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
			color = calc_color_from_dist(cloud.pts_cyl[i].dist, cloud.max, lightness);
		}

		if (render_points) {
			render_point(cloud.pts_cart[i].x, cloud.pts_cart[i].y, color);
		}

		color.a = 128;
		sf::Vertex vertex(sf::Vector2f(
			cloud.pts_cart[i].x * sets_.scale + sets_.origin_x,
			cloud.pts_cart[i].y * sets_.scale + sets_.origin_y),
			color
		);
		vertex_arr.append(vertex);
	}
	window_.draw(vertex_arr);
}

void SFMLGUI::render_point(int x, int y, const Color& color) {
	auto pt = sf::CircleShape(2);
	pt.setOrigin(1, 1);
	pt.setPosition(x * sets_.scale + sets_.origin_x, y * sets_.scale + sets_.origin_y);
	pt.setFillColor(color);
	window_.draw(pt);
}

void SFMLGUI::render_line(float x0, float y0, float x1, float y1, const Color& color) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		render_point(int(x0), int(y0), color);
		x0 += x; y0 += y;
	}
}

void SFMLGUI::render_line(float x0, float y0, float x1, float y1, float max_dist, SFMLGUISettings::Colormap colormap, float lightness) {
	float x = x1 - x0, y = y1 - y0;
	const float max = std::max(std::fabs(x), std::fabs(y));
	x /= max; y /= max;
	for (float n = 0; n < max; n++) {
		float dist = std::hypot(x0 - sets_.origin_x, y0 - sets_.origin_y);
		render_point(int(x0), int(y0), calc_color_from_dist(dist, max_dist, lightness));
		x0 += x; y0 += y;
	}
}

void SFMLGUI::render_mark() {

}

bool SFMLGUI::save_screenshot() {
	return false;
}

float SFMLGUI::calc_scale(float max_dist) {
	return float(sets_.height) * 0.7f / max_dist;
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

	return Color(float(c0.r + c1.r) * lightness, 
		float(c0.g + c1.g) * lightness, 
		float(c0.b + c1.b) * lightness, 
		255
	);
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

	return Color(float(c0.r + c1.r) * lightness,
		float(c0.g + c1.g) * lightness, 
		float(c0.b + c1.b) * lightness, 
		255
	);
}