#include "cloud.h"

//
// PointCyl
//
PointCart PointCyl::to_cart() const {
  int x = int(std::round(dist * std::sin(angle * (acos(-1.0f) / 180.0f))));
  int y = int(std::round(dist * std::cos(angle * (acos(-1.0f) / 180.0f))));
  return {x, y};
}

PointCart PointCyl::to_cart(float scale, float origin_x, float origin_y) const {
  int x = int(std::round(dist * std::sin(angle * (acos(-1.0f) / 180.0f)) * scale)) + origin_x;
  int y = int(std::round(dist * std::cos(angle * (acos(-1.0f) / 180.0f)) * scale)) + origin_y;
  return {x, y};
}