#ifndef __SKILL_H__
#define __SKILL_H__

#include <vector>
using std::vector;

#include "common.h"

enum {
  HP = 10,
  ATK = 20,
  DEF = 30,
};

struct World;
struct Actor;
struct Skill {
  struct Param {
    vector<float>& p_;
    Skill& skill_;
    Param(vector<float>& p, Skill& skill) : p_(p), skill_(skill) {}
    float operator[](int idx) {
      --idx;
      if (idx < 0 || idx >= p_.size()) return 0;
      return p_[idx];
    }
  };
  using Func = void (*)(Actor& src, Actor& targ, Param& p);
  struct Skillcfg {
    int cd_;
    int16_t num_;
    bool samecamp_;
    int8_t rtp_;
    float p1_, p2_;
    vector<float> param_;
  };
  struct Buffcfg {
    int duration_;
  };
  World& world_;
  hashtable<int, Func> func_;
  hashtable<int, Skillcfg> skillcfg_;
  hashtable<int, Buffcfg> buffcfg_;

  Skill(World& world);
  void init_skillfunc();

  void useskill(int64_t id, int skillid);
};

namespace Skillfunc {
float fattr(Actor& actor, int k);
void damage(Actor& targ, float v, Actor& src);
};  // namespace Skillfunc

#endif