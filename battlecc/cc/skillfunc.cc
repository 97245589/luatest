#include "battle.h"
void Skill::initfunc() {
  skillfunc_[100] = [=]() {
    float v = fattr(src_, ATK) * p_[1] - fattr(targ_, DEF);
    damage(v);
  };
  skillfunc_[200] = [=]() {
    addbuff(1);
    float v = targ_->blo_;
    buff_roundend([=]() { addhp(v); });
  };
  skillfunc_[300] = [=]() {
    addbuff(1);
    buffattr({p_[1], p_[2]});
  };
}
