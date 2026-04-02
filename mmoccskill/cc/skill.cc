#include <iostream>
using namespace std;

#include "world.h"

Skill::Skill(World& world) : world_(world) { init_skillfunc(); }

void Skill::useskill(int64_t id, int skillid) {
  auto& actor_ = world_.actor_;
  auto it = actor_.find(id);
  if (it == actor_.end()) return;
  Actor& actor = it->second;
  auto cit = skillcfg_.find(skillid);
  if (cit == skillcfg_.end()) return;
  auto fit = func_.find(skillid);
  if (fit == func_.end()) return;
  Skill::Func func = fit->second;
  if (!func) return;

  Skillcfg& cfg = cit->second;
  vector<int64_t> ids;
  Search search{id, cfg.num_, cfg.samecamp_, cfg.rtp_, cfg.p1_, cfg.p2_};
  world_.aoi_.search(search, ids);
  Skill::Param p(cfg.param_, *this);
  for (int64_t oid : ids) {
    auto oit = actor_.find(oid);
    if (oit == actor_.end()) continue;
    Actor& oactor = oit->second;
    func(actor, oactor, p);
  }
}

namespace Skillfunc {

float fattr(Actor& actor, int k) {
  auto& attrs = actor.attrs_;
  float base = attrs[k];
  float add = attrs[k + 1];
  float per = attrs[k + 2];
  return (base + add) * (1 + per);
}

void damage(Actor& targ, float v, Actor& src) {
  cout << "damage:" << targ.id_ << " " << v << endl;
}

}  // namespace Skillfunc