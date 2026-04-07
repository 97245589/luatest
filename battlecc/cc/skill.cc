#include <iostream>

#include "battle.h"
using namespace std;

Skill::Skill() {
  initfunc();

  targf_["me"] = [=]() { targs_.push_back(src_); };
  targf_["enemy"] = [=]() {
    auto atkorder = battle_->atk_order_;
    int m = src_->blo_ / 100;
    for (Actor* actor : atkorder) {
      int om = actor->blo_ / 100;
      if (m != om) {
        targs_.push_back(actor);
        break;
      }
    }
  };
}

void Skill::useskill(int skillid) {
  targ_ = nullptr;
  p_.params_ = nullptr;
  buff_ = nullptr;
  targs_.clear();

  if (!src_) return;
  auto it = skillcfg_.find(skillid);
  if (it == skillcfg_.end()) return;
  auto& cfg = it->second;
  auto targf = targf_[cfg.targ_];
  if (!targf) return;

  int actionid = skillid / 100;
  actionid = actionid * 100;
  auto ait = skillfunc_.find(actionid);
  if (ait == skillfunc_.end()) return;
  auto action = ait->second;
  if (!action) return;

  p_.params_ = &cfg.tparams_;
  targf();
  if (targs_.empty()) return;

  p_.params_ = &cfg.aparams_;
  for (Actor* targ : targs_) {
    targ_ = targ;
    action();
  }
  src_ = nullptr;
}

void Skill::roundend() {
  auto atkorder = battle_->atk_order_;
  int round = battle_->round_;
  for (Actor* p : atkorder) {
    Actor& actor = *p;
    targ_ = src_ = p;
    for (auto& [id, func] : actor.roundend_) {
      func();
    }
    targ_ = src_ = nullptr;

    auto& buffs = actor.buffs_;
    for (auto it = buffs.begin(); it != buffs.end();) {
      auto& buff = *it;
      if (round >= buff.endtm_) {
        removebuff(actor, buff);
        it = buffs.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void Skill::damage(float v) {
  cout << targ_->blo_ << " " << v << " ";
  cout << src_->blo_ << endl;
}

void Skill::addhp(float v) {
  cout << "addhp:" << targ_->blo_ << " " << v << endl;
}

float Skill::fattr(Actor* actor, int k) {
  if (!actor) return 0;
  auto& attrs = actor->attrs_;
  return attrs[k];
}

void Skill::removebuff(Actor& actor, Buff& buff) {
  cout << "removebuff:" << actor.blo_ << " " << buff.tid_ << endl;
  actor.roundend_.erase(buff.id_);
  for (auto [k, v] : buff.attrs_) {
    actor.attrs_[k] -= v;
  }
}

void Skill::addbuff(int tid) {
  auto& targ = *targ_;
  auto& buffs = targ.buffs_;

  int id = ++targ.idx_;
  Buff buff{.id_ = id, .tid_ = tid, .endtm_ = 1};
  buffs.push_back(buff);
  buff_ = &buffs.back();
}

void Skill::buffattr(const vector<float>& arr) {
  if (!buff_) return;
  auto& targ = *targ_;
  auto& buff = *buff_;

  for (int i = 0; i < arr.size() - 1; i += 2) {
    int k = arr[i];
    float v = arr[i + 1];
    buff.attrs_[k] = v;
    targ.attrs_[k] += v;
  }
}

void Skill::buff_roundend(function<void()> func) {
  if (!buff_) return;
  auto& targ = *targ_;
  auto& buff = *buff_;
  targ.roundend_[buff.id_] = func;
}