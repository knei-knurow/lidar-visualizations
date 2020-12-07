#pragma once
#include <vector>
#include "cloud-grabbers.h"
#ifdef USING_SFML
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#endif

class GUI {
public:
	virtual bool update(const Cloud & cloud) = 0;
};


class TerminalGUI
	: public GUI {
public:
	TerminalGUI();
	virtual bool update(const Cloud& cloud);

private:
};

#ifdef USING_SFML
typedef sf::Color Color;

struct SFMLGUISettings {
	enum Colormap {
		FROM_ANGLE,
		FROM_DIST,

		COLORMAP_COUNT,
	};

	enum PtsDispayMode {
		DOTS_LINES,
		DOTS,
		LINES,

		PTS_DISPLAY_MODE_COUNT,
	};

	enum { SCALE_AUTO = 0 };
	
	bool running = true;
	unsigned width = 1280;
	unsigned height = 720;
	int origin_x = width / 2;
	int origin_y = height / 2;
	float scale = SCALE_AUTO;
	Colormap colormap = FROM_ANGLE;
	PtsDispayMode pts_display_mode = DOTS_LINES;
	float sleep_time_ms = 1000.0f / 30.0f;

	Color color_background = Color(16, 16, 24);
	Color color_grid = Color(20, 20, 34);

	bool render_mouse_ray = true;
	bool render_grid = true;
	unsigned antialiasing = 8;
};

class SFMLGUI
	: public GUI {
public:
	SFMLGUI(const SFMLGUISettings& settings);
	virtual bool update(const Cloud& cloud);

	inline SFMLGUISettings settings() const { return sets_; }
	inline SFMLGUISettings& settings() { return sets_; }

private:
	void handle_input();
	void render_grid();
	void render_cloud_bars(const Cloud& cloud);
	void render_cloud(const Cloud& cloud, float lightness = 1.0f);
	void render_connected_cloud(const Cloud& cloud, float lightness = 1.0f, bool render_points = true);
	void render_point(int x, int y, const Color& color);
	void render_line(float x0, float y0, float x1, float y1, const Color& color);
	void render_line(float x0, float y0, float x1, float y1, float max_dist, SFMLGUISettings::Colormap colormap, float lightness);
	void render_mark();

	bool save_screenshot();

	float calc_scale(float max_dist);
	Color calc_color_from_angle(float v, float lightness = 1.0f);
	Color calc_color_from_dist(float dist, float max, float lightness = 1.0f);

	SFMLGUISettings sets_;
	sf::RenderWindow window_;
};
#endif

