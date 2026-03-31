#include <cmath>

#include "skill.h"
using namespace Skillfunc;
using Param = Skill::Param;

static void s100(Actor& src, Actor& targ, Param& p) { damage(targ, p[1], src); }

void Skill::init_skillfunc() { func_[100] = s100; }