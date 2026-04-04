#include "battle.h"
using namespace Skillfunc;
static void s101(Actor& src, Actor& targ, Param p) {float v = fattr(src,ATK)*p[1]; buff_roundend(targ,src,1,p,[=](Actor& actor, Buff& buff){addhp(actor, v);});}
void Skill::initfunc() {
skillfunc_.insert({101, s101});
}
