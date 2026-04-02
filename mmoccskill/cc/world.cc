#include "world.h"

#include <algorithm>
#include <iostream>

using namespace std;

World::World(int16_t len, int16_t wid)
    : len_(len), wid_(wid), astar_(*this), aoi_(*this), skill_(*this) {}

bool World::inarea(Pos p) {
  if (p.x_ < 0 || p.x_ > len_ - 1) return false;
  if (p.y_ < 0 || p.y_ > wid_ - 1) return false;
  return true;
}

bool World::isblock(Pos p) {
  if (!inarea(p)) return true;
  return block_.find(p) != block_.end();
}

void World::setblock(Pos p, int8_t v) {
  if (!inarea(p)) return;
  if (v == 0) {
    block_.erase(p);
  } else {
    block_[p] = v;
  }
}

void World::correctpos(Actor& actor) {
  if (actor.x_ < 0) actor.x_ = 0;
  if (actor.x_ > len_ - 1) actor.x_ = len_ - 1;
  if (actor.y_ < 0) actor.y_ = 0;
  if (actor.y_ > wid_ - 1) actor.y_ = wid_ - 1;
}

void World::addactor(int64_t id, Actor& actor) {
  auto it = actor_.find(id);
  if (it != actor_.end()) return;
  correctpos(actor);
  actor_[id] = actor;
  aoi_.add(id, actor);
}

void World::delactor(int64_t id) {
  auto ait = actor_.find(id);
  if (ait == actor_.end()) return;
  const Actor& actor = ait->second;
  aoi_.del(id, actor);
  actor_.erase(id);
}

void World::setpos(int64_t id, float fx, float fy, int16_t dx, int16_t dy,
                   vector<int64_t>& adds, vector<int64_t>& dels) {
  auto it = actor_.find(id);
  if (it == actor_.end()) return;
  Actor& actor = it->second;
  Pos bp;
  bp.x_ = actor.x_;
  bp.y_ = actor.y_;
  aoi_.del(id, actor);
  actor.x_ = fx;
  actor.y_ = fy;
  actor.dx_ = dx;
  actor.dy_ = dy;
  correctpos(actor);
  aoi_.add(id, actor);

  Pos np;
  np.x_ = actor.x_;
  np.y_ = actor.y_;
  aoi_.diff(id, bp, np, adds, dels);
}
