#include "guis.h"

//
//	TerminalGUI
//
TerminalGUI::TerminalGUI(bool verbose) {
  verbose_ = verbose;
  count_ = 0;
}

bool TerminalGUI::update(const Cloud& cloud) {
  std::cout << "!\t" << ++count_ << std::endl;
  if (verbose_) {
    for (const auto& pt : cloud.pts_cyl) {
      std::cout << pt.angle << "\t" << pt.dist << std::endl;
    }
  }
  return true;
}

//
//	SFMLGUI
//
#ifdef USING_SFML
SFMLGUI::SFMLGUI(const SFMLGUISettings& settings)
    : cloud_writer_(settings.output_dir) {
  sets_ = settings;

  sf::ContextSettings window_settings;
  window_settings.antialiasingLevel = sets_.antialiasing;

  window_.create(sf::VideoMode(sets_.width, sets_.height), "Lidar",
                 sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize,
                 window_settings);

  for (int i = 0; i < int(StatusKey::COUNT); i++) {
    status_keys_[i] = false;
  }

  screenshots_cnt_ = 0;
}

bool SFMLGUI::update(const Cloud& cloud) {
  if (!sets_.running) {
    window_.close();
    return false;
  }

  handle_input(cloud);
  window_.clear(sets_.color_background);

  if (sets_.render_grid) {
    render_grid();
  }

  if (cloud.size > 0) {
    if (sets_.autoscale) {
      auto new_scale = calc_scale(cloud.max);
      if (new_scale >= sets_.scale * 1.1 || new_scale <= sets_.scale * 0.9)
        sets_.scale = new_scale;
    }

    if (sets_.render_grid) {
      render_cloud_bars(cloud);
    }

    render_front_line(cloud.pts_cart.front().x, cloud.pts_cart.front().y);

    if (sets_.pts_display_mode == SFMLGUISettings::DOTS_LINES) {
      render_connected_cloud(cloud);
    } else if (sets_.pts_display_mode == SFMLGUISettings::DOTS) {
      render_cloud(cloud);
    } else if (sets_.pts_display_mode == SFMLGUISettings::LINES) {
      render_connected_cloud(cloud, 1.0, false);
    }
  }

  render_point(0, 0, Color::Red);

  window_.display();
  window_.setTitle("Scale: 1mm ->" + std::to_string(sets_.scale) + "px");
  sf::sleep(sf::milliseconds(sets_.sleep_time_ms));
  return true;
}

void SFMLGUI::handle_input(const Cloud& cloud) {
  sf::Event event;
  while (window_.pollEvent(event)) {
    if (event.type == sf::Event::Resized) {
      sets_.width = event.size.width;
      sets_.height = event.size.height;
      sets_.origin_x = event.size.width / 2;
      sets_.origin_y = event.size.height / 2;
      sf::FloatRect visible_area(0, 0, event.size.width, event.size.height);
      window_.setView(sf::View(visible_area));
    }
    if (event.type == sf::Event::Closed)
      sets_.running = false;
    if (event.type == sf::Event::KeyPressed) {
      if (event.key.code == sf::Keyboard::Up)
        status_keys_[int(StatusKey::UP)] = true;
      if (event.key.code == sf::Keyboard::Down)
        status_keys_[int(StatusKey::DOWN)] = true;
      if (event.key.code == sf::Keyboard::Left)
        status_keys_[int(StatusKey::LEFT)] = true;
      if (event.key.code == sf::Keyboard::Right)
        status_keys_[int(StatusKey::RIGHT)] = true;

      if (event.key.code == sf::Keyboard::S)
        save_screenshot();
      if (event.key.code == sf::Keyboard::T)
        save_cloud(cloud);

      if (event.key.code == sf::Keyboard::R)
        sets_.render_mouse_ray = !sets_.render_mouse_ray;
      if (event.key.code == sf::Keyboard::C)
        sets_.colormap = SFMLGUISettings::Colormap(
            (sets_.colormap + 1) % SFMLGUISettings::COLORMAP_COUNT);
      if (event.key.code == sf::Keyboard::M)
        sets_.pts_display_mode = SFMLGUISettings::PtsDispayMode(
            (sets_.pts_display_mode + 1) %
            SFMLGUISettings::PTS_DISPLAY_MODE_COUNT);
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
      sets_.autoscale = false;
      if (event.mouseWheelScroll.delta > 0)
        sets_.scale *= 1.25;
      else if (event.mouseWheelScroll.delta < 0)
        sets_.scale *= 0.8;
    } else if (event.type == sf::Event::MouseButtonPressed) {
      if (event.key.code == sf::Mouse::Middle) {
        sets_.autoscale = true;
        sets_.origin_y = sets_.height / 2;
        sets_.origin_x = sets_.width / 2;
      }
    }

    if (event.type == sf::Event::KeyReleased) {
      if (event.key.code == sf::Keyboard::Up)
        status_keys_[int(StatusKey::UP)] = false;
      if (event.key.code == sf::Keyboard::Down)
        status_keys_[int(StatusKey::DOWN)] = false;
      if (event.key.code == sf::Keyboard::Left)
        status_keys_[int(StatusKey::LEFT)] = false;
      if (event.key.code == sf::Keyboard::Right)
        status_keys_[int(StatusKey::RIGHT)] = false;
    }
  }

  if (status_keys_[int(StatusKey::UP)])
    sets_.origin_y += 25;
  if (status_keys_[int(StatusKey::DOWN)])
    sets_.origin_y -= 25;
  if (status_keys_[int(StatusKey::LEFT)])
    sets_.origin_x += 25;
  if (status_keys_[int(StatusKey::RIGHT)])
    sets_.origin_x -= 25;
}

void SFMLGUI::render_grid() {
  for (int i = 0; i <= 30; i++) {
    auto circle = sf::CircleShape(1000 * i * sets_.scale, 64);
    circle.setOrigin(circle.getRadius() - 1, circle.getRadius() - 1);
    circle.setPosition(sets_.origin_x, sets_.origin_y);
    circle.setFillColor(Color::Transparent);
    circle.setOutlineColor(sets_.color_grid);
    if (i % 10 == 0)
      circle.setOutlineThickness(4);
    else
      circle.setOutlineThickness(1);
    window_.draw(circle);
  }
}

void SFMLGUI::render_cloud_bars(const Cloud& cloud) {
  if (cloud.size == 0)
    return;

  unsigned max_width = 80;
  for (int j = 0; j < sets_.height; j++) {
    float dist = cloud.pts_cyl[size_t(j * cloud.size / sets_.height)].dist;

    int width = int(std::round(dist / cloud.max * max_width));

    Color color;
    if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
      color = calc_color_from_angle(float(j * cloud.size / sets_.height) /
                                    float(cloud.size));
    } else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
      color = calc_color_from_dist(
          cloud.pts_cyl[size_t(j * cloud.size / sets_.height)].dist, cloud.max,
          1.0);
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
    } else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
      color = calc_color_from_dist(cloud.pts_cyl[i].dist, cloud.max, lightness);
    }
    render_point(cloud.pts_cart[i].x, cloud.pts_cart[i].y, color);
  }
}

void SFMLGUI::render_connected_cloud(const Cloud& cloud,
                                     float lightness,
                                     bool render_points) {
  if (cloud.size == 0)
    return;

  sf::VertexArray vertex_arr(sf::PrimitiveType::LinesStrip);
  for (int i = 0; i < cloud.size; i++) {
    Color color;
    if (sets_.colormap == SFMLGUISettings::FROM_ANGLE) {
      color = calc_color_from_angle(float(i) / float(cloud.size), lightness);
    } else if (sets_.colormap == SFMLGUISettings::FROM_DIST) {
      color = calc_color_from_dist(cloud.pts_cyl[i].dist, cloud.max, lightness);
    }

    if (render_points) {
      render_point(cloud.pts_cart[i].x, cloud.pts_cart[i].y, color);
    }

    color.a = 128;
    sf::Vertex vertex(
        sf::Vector2f(cloud.pts_cart[i].x * sets_.scale + sets_.origin_x,
                     cloud.pts_cart[i].y * sets_.scale + sets_.origin_y),
        color);
    vertex_arr.append(vertex);
  }
  vertex_arr.append(vertex_arr[0]);
  window_.draw(vertex_arr);
}

void SFMLGUI::render_point(int x, int y, const Color& color) {
  auto pt = sf::CircleShape(2);
  pt.setOrigin(1, 1);
  pt.setPosition(x * sets_.scale + sets_.origin_x,
                 y * sets_.scale + sets_.origin_y);
  pt.setFillColor(color);
  window_.draw(pt);
}

void SFMLGUI::render_front_line(int x, int y) {
  sf::Vertex line[] = {
      sf::Vertex(sf::Vector2f(sets_.origin_x, sets_.origin_y),
                 sets_.color_grid),
      sf::Vertex(sf::Vector2f(x * sets_.scale + sets_.origin_x,
                              y * sets_.scale + sets_.origin_y),
                 sets_.color_grid)};
  window_.draw(line, 2, sf::Lines);
}

bool SFMLGUI::save_screenshot() {
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d.%m.%Y-%H.%M.%S");

  auto filename = sets_.output_dir + "/screenshot-" +
                  std::to_string(++screenshots_cnt_) + "-" + oss.str() + ".png";

  sf::Texture texture;
  texture.create(sets_.width, sets_.height);
  texture.update(window_);

  if (!texture.copyToImage().saveToFile(filename)) {
    std::cerr << "ERROR: Unable to save screenshot." << std::endl;
    return false;
  } else {
    std::cout << "Screenshot saved: " << filename << std::endl;
    return true;
  }
}

bool SFMLGUI::save_cloud(const Cloud& cloud) {
  return cloud_writer_.write(cloud);
}

float SFMLGUI::calc_scale(float max_dist) {
  return float(sets_.height) * 0.7f / max_dist;
}

Color SFMLGUI::calc_color_from_angle(float v, float lightness) {
  Color c0, c1;
  if (v >= 0 && v <= 0.33f) {
    c0 = Color(0, 255, 255);
    c1 = Color(255, 0, 255);
  } else if (v <= 0.66f) {
    v -= 0.33f;
    c0 = Color(255, 255, 0);
    c1 = Color(0, 255, 255);
  } else if (v <= 1.0f) {
    v -= 0.66f;
    c0 = Color(255, 0, 255);
    c1 = Color(255, 255, 0);
  } else {
    return Color(255, 255, 255);
  }

  c0.r *= float(v) / 0.34f;
  c0.g *= float(v) / 0.34f;
  c0.b *= float(v) / 0.34f;
  c1.r *= 1.0f - float(v) / 0.34f;
  c1.g *= 1.0f - float(v) / 0.34f;
  c1.b *= 1.0f - float(v) / 0.34f;

  return Color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness,
               float(c0.b + c1.b) * lightness, 255);
}

Color SFMLGUI::calc_color_from_dist(float dist, float max, float lightness) {
  float v = dist / max;
  Color c0, c1;
  if (v >= 0 && v <= 0.33f) {
    c0 = Color(255, 255, 0);
    c1 = Color(255, 0, 0);
  } else if (v <= 0.66f) {
    v -= 0.33f;
    c0 = Color(0, 255, 0);
    c1 = Color(255, 255, 0);
  } else if (v <= 1.0f) {
    v -= 0.66f;
    c0 = Color(0, 0, 255);
    c1 = Color(0, 255, 0);
  } else {
    return Color(255, 255, 255);
  }

  c0.r *= float(v) / 0.34f;
  c0.g *= float(v) / 0.34f;
  c0.b *= float(v) / 0.34f;
  c1.r *= 1.0f - float(v) / 0.34f;
  c1.g *= 1.0f - float(v) / 0.34f;
  c1.b *= 1.0f - float(v) / 0.34f;

  return Color(float(c0.r + c1.r) * lightness, float(c0.g + c1.g) * lightness,
               float(c0.b + c1.b) * lightness, 255);
}
#endif
