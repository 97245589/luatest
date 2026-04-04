#ifndef __BATTLE_H__
#define __BATTLE_H__

#include "skill.h"

struct Actor {
  int8_t lo_;
  int16_t blo_;
  uint16_t buffidx_;
  hashtable<int, float> attrs_;
  hashtable<int, int> active_skills_;
  hashtable<int, int> passive_skills_;
  hashtable<int, Buff> buffs_;
  float fattr(int k);
};

struct Battle {
  int8_t round_;
  Skill skill_;
  vector<Actor> atk_, def_;
  vector<Actor*> atkorder_;

  Battle() : skill_(*this) {}
  void roundend(Actor&);
  void round();
  void start();
};

#endif