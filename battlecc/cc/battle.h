#ifndef __BATTLE_H__
#define __BATTLE_H__

#include "skill.h"

struct Actor {
  int8_t lo_;
  int16_t blo_;
  hashtable<int, float> attrs_;

  uint16_t idx_;
  vector<Buff> buffs_;
  hashtable<int, std::function<void()>> roundend_;
  hashtable<int, hashtable<int, std::function<void()>>> eventfunc_;
};

struct Battle {
  int8_t round_;
  vector<Actor> atk_, def_;
  vector<Actor*> atk_order_;
  Skill skill_;

  Battle();
  void round();
  void start();
};

#endif