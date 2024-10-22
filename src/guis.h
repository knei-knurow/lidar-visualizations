#pragma once
#include <vector>
#include "cloud-writers.h"
#include "cloud.h"
#ifdef USING_SFML
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#endif

enum class GUIType {
  TERMINAL,
  SFML,
};

class GUI {
 public:
  virtual bool update(const Cloud& cloud) = 0;
};

class TerminalGUI : public GUI {
 public:
  TerminalGUI(bool verbose = true);
  virtual bool update(const Cloud& cloud);

 private:
  bool verbose_;
  size_t count_;
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

  bool running = true;
  unsigned width = 1280;
  unsigned height = 720;
  int origin_x = width / 2;
  int origin_y = height / 2;
  float scale = 0.2;
  bool autoscale = true;
  Colormap colormap = FROM_ANGLE;
  PtsDispayMode pts_display_mode = DOTS_LINES;
  float sleep_time_ms = 1000.0f / 30.0f;

  Color color_background = Color(16, 16, 24);
  Color color_grid = Color(40, 40, 50);

  bool render_mouse_ray = true;
  bool render_grid = true;
  bool bold_mode = false;
  unsigned antialiasing = 8;

  std::string output_dir = ".";
};

class SFMLGUI : public GUI {
 public:
  SFMLGUI(const SFMLGUISettings& settings);
  virtual bool update(const Cloud& cloud);

  inline SFMLGUISettings settings() const { return sets_; }
  inline SFMLGUISettings& settings() { return sets_; }

  bool save_screenshot();
  bool save_cloud(const Cloud& cloud);

 private:
  enum class StatusKey {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    COUNT,
  };

  void handle_input(const Cloud& cloud);
  void render_grid();
  void render_cloud_bars(const Cloud& cloud);
  void render_cloud(const Cloud& cloud, float lightness = 1.0f);
  void render_connected_cloud(const Cloud& cloud,
                              float lightness = 1.0f,
                              bool render_points = true);
  void render_point(int x, int y, const Color& color);
  void render_front_line(int x, int y);

  float calc_scale(float max_dist);
  Color calc_color_from_angle(float v, float lightness = 1.0f);
  Color calc_color_from_dist(float dist, float max, float lightness = 1.0f);

  SFMLGUISettings sets_;
  sf::RenderWindow window_;
  bool status_keys_[int(StatusKey::COUNT)];
  size_t screenshots_cnt_;
  CloudFileWriter cloud_writer_;
};
#endif
