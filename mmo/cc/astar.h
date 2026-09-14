#ifndef __ASTAR_H__
#define __ASTAR_H__

#include <vector>
using std::vector;
#include "common.h"

struct Map;
struct Astar {
  struct State {
    int16_t x_, y_;
    int cost_, weigh_;
    bool operator<(const State& rhs) const { return weigh_ < rhs.weigh_; }
  };

  Map& map_;
  Astar(Map& m) : map_(m) {}

  bool impassable(Pos p);
  void find(Pos s, Pos e, vector<Pos>& ret, bool quick);
};

#endif