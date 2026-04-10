#include <iostream>

#include "battle.h"
using namespace std;

Skill::Skill() {
  targfunc();
  initfunc();
}

void Skill::targfunc() {
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
      if (round >= it->endtm_) {
        removebuff(actor, *it);
        it = buffs.erase(it);
      } else {
        break;
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

void Skill::removebuff(Actor& actor, const Buff& buff) {
  // cout << "removebuff:" << actor.blo_ << " " << buff.tid_ << endl;
  actor.roundend_.erase(buff.id_);
  for (auto [k, v] : buff.attrs_) {
    actor.attrs_[k] -= v;
  }
  int event = buff.event_;
  if (event > 0) {
    actor.eventfunc_[event].erase(buff.id_);
  }
}

bool Skill::initbuff(int tid, Buff& buff) {
  auto& targ = *targ_;
  buff.id_ = ++targ.idx_;
  buff.tid_ = tid;
  buff.endtm_ = 1;
  return true;
}

void Skill::buffattr(int tid, const vector<float>& arr) {
  Buff buff{};
  bool ok = initbuff(tid, buff);
  if (!ok) return;
  for (int i = 0; i < arr.size() - 1; i += 2) {
    int k = arr[i];
    float v = arr[i + 1];
    buff.attrs_[k] = v;
    targ_->attrs_[k] += v;
  }
  targ_->buffs_.insert(std::move(buff));
}

void Skill::buff_roundend(int tid, function<void()> func) {
  Buff buff{};
  bool ok = initbuff(tid, buff);
  if (!ok) return;
  targ_->roundend_[buff.id_] = func;
  targ_->buffs_.insert(std::move(buff));
}

void Skill::buffevent(int tid, int event, function<void()> func) {
  Buff buff{};
  bool ok = initbuff(tid, buff);
  if (!ok) return;
  targ_->eventfunc_[event][buff.id_] = func;
  buff.event_ = event;
  targ_->buffs_.insert(std::move(buff));
}

void Skill::trigger(Actor* p, int event, Eventdata data) {
  auto& funcs = p->eventfunc_[event];
  e_ = data;
  src_ = targ_ = p;
  for (auto& [buffid, func] : funcs) {
    func();
  }
  src_ = targ_ = nullptr;
}