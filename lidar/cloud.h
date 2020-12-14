#pragma once
#include <cmath>
#include <vector>

enum class CoordSystem {
  CART,
  CYL,
};

struct PointCart {
  int x = 0;
  int y = 0;
};

struct PointCyl {
  float angle = 0;
  float dist = 0;

  PointCart to_cart() const;
  PointCart to_cart(float scale, float origin_x, float origin_y) const;
};

struct Cloud {
  std::vector<PointCyl> pts_cyl;
  std::vector<PointCart> pts_cart;
  float min = std::numeric_limits<float>::max();
  float max = 0;
  size_t max_idx = -1;
  size_t min_idx = -1;
  size_t size = 0;
};
