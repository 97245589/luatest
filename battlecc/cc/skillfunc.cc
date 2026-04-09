#include "battle.h"

void Skill::initfunc() {
  skillfunc_[100] = [=]() {
    float v = fattr(src_, ATK) * p_[1] - fattr(targ_, DEF);
    damage(v);
  };

  skillfunc_[200] = [=]() {
    float v = fattr(targ_, ATK) * p_[1];
    buff_roundend(10, [=]() { addhp(v); });
  };

  skillfunc_[300] = [=]() { buffattr(10, {p_[1], p_[2]}); };

  skillfunc_[400] = [=]() {
    float v = p_[1];
    buffevent(10, ESKILL, [=]() { addhp(v); });
  };

  skillfunc_[500] = [=]() {
    auto p = p_;
    buff_roundend(10, [=]() { float v = p[1]; });
  };
}
