#ifndef __SKILL_H__
#define __SKILL_H__

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <functional>
#include <string>
#include <vector>
using std::function;
using std::string;
using std::vector;

#define hashtable __gnu_pbds::cc_hash_table

enum {
  HP = 10,
  ATK = 20,
  DEF = 30,

  ESKILL = 1,
  EATk,
  EATKED
};

struct Battle;
struct Actor;

struct Skillcfg {
  string targ_;
  vector<float> tparams_;
  vector<float> aparams_;
};

struct Buffcfg {};

struct Param {
  vector<float>* params_;
  float operator[](size_t idx) {
    if (!params_) return 0;
    auto& p = *params_;
    --idx;
    if (idx < 0 || idx >= p.size()) return 0;
    return p[idx];
  }
};

struct Buff {
  int id_;
  int tid_;
  int64_t endtm_;
  int event_;
  hashtable<int, float> attrs_;
};

struct Skill {
  Battle* battle_;
  Actor* src_;
  Actor* targ_;
  Buff* buff_;
  Param p_;
  vector<Actor*> targs_;

  hashtable<int, function<void()>> skillfunc_;
  hashtable<string, function<void()>> targf_;
  hashtable<int, Skillcfg> skillcfg_;
  hashtable<int, Buffcfg> buffcfg_;

  Skill();
  void initfunc();
  void useskill(int skillid);
  void roundend();

  void damage(float v);
  void addhp(float v);
  float fattr(Actor* actor, int k);
  void removebuff(Actor& actor, Buff& buff);
  void addbuff(int tid);
  void buffattr(const vector<float>&);
  void buff_roundend(function<void()>);
};

#endif