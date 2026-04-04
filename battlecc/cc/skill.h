#ifndef __SKILL_H__
#define __SKILL_H__

#include <cmath>
#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <functional>
#include <string>
#include <vector>
using std::function;
using std::string;
using std::vector;
#define hashtable __gnu_pbds::cc_hash_table

enum { HP = 10, ATK = 20, DEF = 30 };

struct Skill;
struct Buff;
struct Battle;
struct Actor;

struct Param {
  vector<float>& params_;
  Skill& skill_;
  Param(vector<float>& p, Skill& s) : params_(p), skill_(s) {}
  float operator[](int idx) {
    --idx;
    if (idx < 0 || idx >= params_.size()) return 0;
    return params_[idx];
  }
};

using Roundend_func = function<void(Actor&, Buff&)>;
struct Buff {
  int tid_;
  int end_;
  hashtable<int, float> attrs_;
  Roundend_func roundend_;
};

struct Skill {
  struct Skillcfg {
    string targ_;
    vector<float> tparams_;
    vector<float> params_;
  };
  struct Buffcfg {
    uint16_t max_stack_;
  };
  using Skillfunc = void (*)(Actor&, Actor&, Param);
  using Targfunc = vector<Actor*> (*)(Actor&, Param);
  hashtable<int, Skillcfg> skillcfg_;
  hashtable<int, Skillfunc> skillfunc_;
  hashtable<string, Targfunc> targfunc_;
  hashtable<int, Buffcfg> buffcfg_;
  Battle& battle_;

  Skill(Battle& b);
  void initfunc();
  void useskill(Actor& src, int skillid);
};

namespace Skillfunc {
float fattr(Actor& actor, int k);
void damage(Actor& targ, float v, Actor& src, Param p);
void addhp(Actor& targ, float v);

Buff* addbuff(Actor& targ, Actor& src, int bufftid, Param p);
void removebuff(Actor& targ, int buffid, Buff& buff);
void buffattr(Actor& targ, Actor& src, int bufftid, Param p);
void buff_roundend(Actor& targ, Actor& src, int bufftid, Param p,
                   Roundend_func func);
};  // namespace Skillfunc

#endif