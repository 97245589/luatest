#ifndef __RANGE_H__
#define __RANGE_H__

#include <cmath>

struct Range {
  struct Vec {
    float x_, y_;
    float m() { return sqrt(x_ * x_ + y_ * y_); }
  };
  static float rad(float ang) { return ang / 180 * M_PI; }
  static float mul(Vec v1, Vec v2) { return v1.x_ * v2.x_ + v1.y_ * v2.y_; }
  static float cosv_2vec(Vec v1, Vec v2) {
    return mul(v1, v2) / (v1.m() * v2.m());
  }
  static float cosv_3vec(Vec v1, Vec v2, Vec v3) {
    Vec v12{.x_ = v2.x_ - v1.x_, .y_ = v2.y_ - v1.y_};
    Vec v13{.x_ = v3.x_ - v1.x_, .y_ = v3.y_ - v1.y_};
    return cosv_2vec(v12, v13);
  }
  static bool circle(Vec p, float r, Vec p1) {
    float dx = p1.x_ - p.x_;
    float dy = p1.y_ - p.y_;
    return r * r >= dx * dx + dy * dy;
  }
  static bool sector(Vec p, Vec d, float r, float ang, Vec p1) {
    if (!circle(p, r, p1)) return false;
    Vec dp{.x_ = p.x_ + d.y_, .y_ = p.y_ + d.y_};
    float cosv = cosv_3vec(p, dp, p1);
    float cosa = cos(rad(ang));
    return cosv >= cosa;
  }
  static bool rectangle(Vec p, Vec d, float l, float w, Vec p1) {
    Vec dp{.x_ = p.x_ + d.y_, .y_ = p.y_ + d.y_};
    float cosv = cosv_3vec(p, dp, p1);
    Vec dis{.x_ = p1.x_ - p.x_, .y_ = p1.y_ - p.y_};
    float dism = dis.m();
    float rx = dism * cosv;
    float ry = dism * sin(acos(cosv));
    if (0 <= rx && rx <= l && 0 <= ry && ry <= w) return true;
    return false;
  }
};

#endif