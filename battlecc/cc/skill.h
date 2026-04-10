#ifndef __SKILL_H__
#define __SKILL_H__

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <functional>
#include <random>
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

  ESKILL = 10,
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
  float operator[](size_t idx) const {
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
  int16_t event_;
  int16_t endtm_;
  hashtable<int, float> attrs_;
  bool operator<(const Buff& rhs) const { return endtm_ < rhs.endtm_; }
};

union Eventdata {
  struct Eskill {
    int skillid_;
  };
  struct Eatk {
    Actor* src_;
    Actor* targ_;
    float val_;
  };
  struct Eatked {
    Actor* src_;
    Actor* targ_;
    float val_;
  };
  Eskill eskill_;
  Eatk eatk_;
  Eatked eatked_;
};

struct Skill {
  Battle* battle_;
  Actor* src_;
  Actor* targ_;
  Param p_;
  vector<Actor*> targs_;
  Eventdata e_;
  std::mt19937 rng_;
  std::uniform_real_distribution<float> dist_;

  hashtable<int, function<void()>> skillfunc_;
  hashtable<string, function<void()>> targf_;
  hashtable<int, Skillcfg> skillcfg_;
  hashtable<int, Buffcfg> buffcfg_;

  Skill();
  void initfunc();
  void targfunc();
  void useskill(int skillid);

  bool rand(float v);

  void damage(float v);
  void addhp(float v);
  float fattr(Actor* actor, int k);
  void removebuff(Actor& actor, const Buff& buff);

  bool initbuff(int tid, Buff& buff);
  void buffattr(int tid, const vector<float>&);

  void buff_roundend(int tid, function<void()>);
  void roundend();

  void buffevent(int tid, int event, function<void()>);
  void trigger(Actor* p, int event, Eventdata data);
};

#endif