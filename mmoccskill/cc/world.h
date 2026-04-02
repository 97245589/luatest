#ifndef __WORLD_H__
#define __WORLD_H__

#include <set>
#include <vector>
using std::multiset;
using std::vector;

#include "aoi.h"
#include "astar.h"
#include "common.h"
#include "skill.h"

struct Actor {
  int64_t id_;
  float x_, y_;
  int16_t dx_, dy_;
  int8_t camp_;
  hashtable<int, float> attrs_;
  hashtable<int, int64_t> skills_;
};

struct World {
  int16_t len_, wid_;
  hashtable<int64_t, Actor> actor_;
  hashtable<Pos, int8_t, Pos> block_;
  Astar astar_;
  Aoi aoi_;
  Skill skill_;

  World(int16_t len, int16_t wid);

  inline bool inarea(Pos);
  bool isblock(Pos);
  void setblock(Pos, int8_t v);

  void correctpos(Actor&);
  void addactor(int64_t id, Actor& actor);
  void delactor(int64_t id);
  void setpos(int64_t id, float x, float y, int16_t dx, int16_t dy,
              vector<int64_t>& adds, vector<int64_t>& dels);
};

#endif