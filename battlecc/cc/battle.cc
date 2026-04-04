#include "battle.h"

#include <algorithm>
using namespace std;

float Actor::fattr(int k) {
  float base = attrs_[k];
  float add = attrs_[k + 1];
  float per = attrs_[k + 2];
  return (base + add) * (1 + per);
}

void Battle::roundend(Actor& actor) {
  auto& buffs = actor.buffs_;
  for (auto& [id, buff] : buffs) {
    if (buff.roundend_) {
      buff.roundend_(actor, buff);
    }
    if (buff.end_ > 0 && round_ >= buff.end_) {
      Skillfunc::removebuff(actor, id, buff);
    }
  }
}

void Battle::round() {
  ++round_;
  atkorder_.clear();
  for (int i = 0; i < 3; ++i) {
    if (i >= atk_.size() || i >= def_.size()) break;
    atkorder_.push_back(&atk_[i]);
    atkorder_.push_back(&def_[i]);
  }
  for (Actor* pactor : atkorder_) {
    skill_.useskill(*pactor, 101);
  }
  for (Actor* pactor : atkorder_) {
    roundend(*pactor);
  }
}

void Battle::start() {
  round_ = 0;
  for (Actor& actor : atk_) {
    actor.blo_ = actor.lo_ + 100;
  }
  for (Actor& actor : def_) {
    actor.blo_ = actor.lo_ + 200;
  }
  auto cmp = [](const Actor& lhs, const Actor& rhs) {
    return lhs.blo_ < rhs.blo_;
  };
  sort(atk_.begin(), atk_.end(), cmp);
  sort(def_.begin(), def_.end(), cmp);

  for (int i = 0; i < 10; ++i) {
    round();
  }

  atk_.clear();
  def_.clear();
  atkorder_.clear();
}