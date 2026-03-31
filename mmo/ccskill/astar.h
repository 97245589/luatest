#ifndef __ASTAR_H__
#define __ASTAR_H__

#include <vector>
using std::vector;
#include "common.h"

struct World;
struct Astar {
  struct State {
    int16_t x_, y_;
    int cost_, weigh_;
    bool operator<(const State& rhs) const { return weigh_ < rhs.weigh_; }
  };

  World& world_;
  Astar(World& world) : world_(world) {}

  bool isblock(Pos p);
  void find(Pos s, Pos e, vector<Pos>& ret, bool quick);
};

#endif