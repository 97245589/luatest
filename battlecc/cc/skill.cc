#include <iostream>
using namespace std;

#include "battle.h"

Skill::Skill(Battle& b) : battle_(b) {
  initfunc();
  skillcfg_[101] = {
      .targ_ = "me",
      .params_ = {2},
  };
  targfunc_["enemy"] = [](Actor& src, Param p) {
    vector<Actor*> ret;
    auto& battle = p.skill_.battle_;
    auto& atkorder = battle.atkorder_;
    int camp = src.blo_ / 100;
    for (Actor* pactor : atkorder) {
      int pcamp = pactor->blo_ / 100;
      if (camp != pcamp) {
        ret.push_back(pactor);
        break;
      }
    }
    return ret;
  };

  targfunc_["me"] = [](Actor& src, Param p) {
    vector<Actor*> ret;
    ret.push_back(&src);
    return ret;
  };
}

void Skill::useskill(Actor& src, int skillid) {
  auto cfg_it = skillcfg_.find(skillid);
  if (cfg_it == skillcfg_.end()) return;
  Skillcfg& cfg = cfg_it->second;
  auto targit = targfunc_.find(cfg.targ_);
  if (targit == targfunc_.end()) return;
  int fid = skillid / 100 * 100 + 1;
  auto funcit = skillfunc_.find(fid);
  if (funcit == skillfunc_.end()) return;
  Param pt(cfg.tparams_, *this);
  vector<Actor*> ret = targit->second(src, pt);
  for (Actor* ptarg : ret) {
    Param ps(cfg.params_, *this);
    funcit->second(src, *ptarg, ps);
  }
}

namespace Skillfunc {

float fattr(Actor& actor, int k) { return actor.fattr(k); }

void damage(Actor& targ, float v, Actor& src, Param p) {
  cout << "damage:" << targ.blo_ << " " << v << " ";
  cout << src.blo_ << endl;
}

void addhp(Actor& targ, float v) {
  cout << "addhp:" << targ.blo_ << " " << v << endl;
}

Buff* addbuff(Actor& targ, Actor& src, int bufftid, Param p) {
  Skill& skill = p.skill_;
  int buffid = targ.buffidx_++;
  Buff buff;
  buff.tid_ = bufftid;
  buff.end_ = 1;
  auto& buffs = targ.buffs_;
  buffs[buffid] = buff;
  return &buffs[buffid];
}

void removebuff(Actor& targ, int buffid, Buff& buff) {
  for (auto [k, v] : buff.attrs_) {
    targ.attrs_[k] -= v;
  }
  auto& buffs = targ.buffs_;
  buffs.erase(buffid);
}

void buffattr(Actor& targ, Actor& src, int bufftid, Param p) {
  Buff* pbuff = addbuff(targ, src, bufftid, p);
  if (!pbuff) return;
  Buff& buff = *pbuff;
  vector<float>& params = p.params_;
  for (int i = 0; i < params.size() - 1; i += 2) {
    int k = params[i];
    float v = params[i + 1];
    buff.attrs_[k] = v;
    targ.attrs_[k] += v;
  }
}

void buff_roundend(Actor& targ, Actor& src, int bufftid, Param p,
                   Roundend_func func) {
  Buff* pbuff = addbuff(targ, src, bufftid, p);
  if (!pbuff) return;
  Buff& buff = *pbuff;
  buff.roundend_ = func;
}

}  // namespace Skillfunc