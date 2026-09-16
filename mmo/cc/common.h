#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdint>
#include "parallel_hashmap/phmap.h"
#define hashmap phmap::flat_hash_map
#define hashset phmap::flat_hash_set

struct Pos {
  int16_t x_, y_;
  bool operator<(Pos rhs) const {
    return x_ * x_ + y_ * y_ < rhs.x_ * rhs.x_ + rhs.y_ * rhs.y_;
  }

  bool operator==(Pos rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
  size_t operator()(Pos p) const {
    int v = p.x_ << 16 | p.y_;
    return std::hash<int>()(v);
  }

  float length() { return sqrt(x_ * x_ + y_ * y_); }
};

#endif