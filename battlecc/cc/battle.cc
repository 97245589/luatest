#include "battle.h"

Battle::Battle() { skill_.battle_ = this; }

void Battle::round() {
  atk_order_.clear();
  for (Actor& actor : atk_) {
    atk_order_.push_back(&actor);
  }
  for (Actor& actor : def_) {
    atk_order_.push_back(&actor);
  }

  for (Actor* pactor : atk_order_) {
    skill_.src_ = pactor;
    skill_.useskill(200);
  }
  skill_.roundend();
}

void Battle::start() {
  for (Actor& actor : atk_) {
    actor.blo_ = 100 + actor.lo_;
  }
  for (Actor& actor : def_) {
    actor.blo_ = 200 + actor.lo_;
  }

  for (int i = 0; i < 1; ++i) {
    round_ = i + 1;
    round();
  }
}