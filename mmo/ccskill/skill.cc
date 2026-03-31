#include <iostream>
using namespace std;

#include "world.h"

Skill::Skill() {
  init_skillcfg();
  init_buffcfg();
  init_skillfunc();
}

void Skill::init_skillcfg() {
  skillcfg_[100] = {
      .num_ = 1000, .samecamp_ = false, .rtp_ = 1, .p1_ = 5, .param_ = {10}};
}

void Skill::init_buffcfg() {}

namespace Skillfunc {

float fattr(Actor& actor, int k) {
  auto& attrs = actor.attrs_;
  float base = attrs[k];
  float add = attrs[k + 1];
  float per = attrs[k + 2];
  return (base + add) * (1 + per);
}

void damage(Actor& targ, float v, Actor& src) {
  cout << "damage:" << targ.id_ << " " << v << endl;
}

}  // namespace Skillfunc