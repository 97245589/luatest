#include "world.h"

#include <algorithm>
#include <iostream>

using namespace std;

World::World(int16_t len, int16_t wid) : len_(len), wid_(wid), astar_(*this) {}

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
  addaoi(id, actor);
}

void World::delactor(int64_t id) {
  auto ait = actor_.find(id);
  if (ait == actor_.end()) return;
  const Actor& actor = ait->second;
  delaoi(id, actor);
  actor_.erase(id);
}

void World::addaoi(int64_t id, const Actor& actor) {
  Pos g;
  g.x_ = actor.x_ / AOILEN;
  g.y_ = actor.y_ / AOILEN;
  aoi_[g].insert(id);
}

void World::delaoi(int64_t id, const Actor& actor) {
  Pos g;
  g.x_ = actor.x_ / AOILEN;
  g.y_ = actor.y_ / AOILEN;
  auto it = aoi_.find(g);
  if (it == aoi_.end()) return;
  auto& ids = it->second;
  ids.erase(id);
  if (ids.empty()) aoi_.erase(g);
}

static const vector<World::Pos> DIR = {{0, 0},  {0, 1},  {0, -1},
                                       {1, 0},  {-1, 0}, {1, 1},
                                       {1, -1}, {-1, 1}, {-1, -1}};
void World::traversal_area(Pos p, function<void(Pos, uset&)> cb) {
  for (Pos d : DIR) {
    Pos ng;
    ng.x_ = p.x_ / AOILEN + d.x_;
    ng.y_ = p.y_ / AOILEN + d.y_;
    auto it = aoi_.find(ng);
    if (it == aoi_.end()) continue;
    auto& ids = it->second;
    cb(ng, ids);
  }
}

void World::setpos(int64_t id, float fx, float fy, int16_t dx, int16_t dy,
                   vector<int64_t>& adds, vector<int64_t>& dels) {
  auto it = actor_.find(id);
  if (it == actor_.end()) return;
  Actor& actor = it->second;
  Pos bp;
  bp.x_ = actor.x_;
  bp.y_ = actor.y_;
  delaoi(id, actor);
  actor.x_ = fx;
  actor.y_ = fy;
  actor.dx_ = dx;
  actor.dy_ = dy;
  correctpos(actor);
  addaoi(id, actor);

  Pos np;
  np.x_ = actor.x_;
  np.y_ = actor.y_;
  aoidiff(id, bp, np, adds, dels);
}

void World::aoidiff(int64_t aid, Pos bp, Pos np, vector<int64_t>& adds,
                    vector<int64_t>& dels) {
  int16_t bgx = bp.x_ / AOILEN;
  int16_t bgy = bp.y_ / AOILEN;
  int16_t ngx = np.x_ / AOILEN;
  int16_t ngy = np.y_ / AOILEN;

  auto coincide = [](Pos g, int16_t gx, int16_t gy) {
    if (g.x_ < gx - 1) return false;
    if (g.x_ > gx + 1) return false;
    if (g.y_ < gy - 1) return false;
    if (g.y_ > gy + 1) return false;
    return true;
  };

  traversal_area(np, [&](Pos g, uset& ids) {
    if (coincide(g, bgx, bgy)) return;
    for (int64_t id : ids) {
      if (adds.size() >= AOIMAX) return;
      if (id != aid) adds.push_back(id);
    }
  });
  traversal_area(bp, [&](Pos g, uset& ids) {
    if (coincide(g, ngx, ngy)) return;
    for (int64_t id : ids) {
      if (dels.size() >= AOIMAX * 2) return;
      dels.push_back(id);
    }
  });
}

void World::areaids(int64_t id, vector<int64_t>& ret) {
  auto it = actor_.find(id);
  if (it == actor_.end()) return;
  ret.reserve(32);
  Actor& actor = it->second;
  Pos p;
  p.x_ = actor.x_;
  p.y_ = actor.y_;
  traversal_area(p, [&](Pos g, uset& ids) {
    for (int64_t id : ids) {
      if (ret.size() >= AOIMAX) return;
      ret.push_back(id);
    }
  });
}

void World::search(const Search& info, vector<int64_t>& ret) {
  ret.reserve(32);
  int64_t id = info.id_;
  bool samecamp = info.samecamp_;
  auto ait = actor_.find(id);
  if (ait == actor_.end()) return;
  Rangefunc rangefunc = range_func(info.rtp_, info.p1_, info.p2_);
  if (!rangefunc) return;
  vector<int64_t> ids;
  areaids(id, ids);
  Actor& actor = ait->second;
  Rvec p{.x_ = actor.x_, .y_ = actor.y_};
  Rvec d;
  d.x_ = actor.dx_;
  d.y_ = actor.dy_;
  for (int64_t oid : ids) {
    auto oit = actor_.find(oid);
    if (oit == actor_.end()) continue;
    Actor& oactor = oit->second;
    if (samecamp && actor.camp_ != oactor.camp_) continue;
    if (!samecamp && actor.camp_ == oactor.camp_) continue;
    Rvec op{.x_ = oactor.x_, .y_ = oactor.y_};
    if (rangefunc(p, d, op)) ret.push_back(oid);
  }
}

World::Rangefunc World::range_func(int8_t rtp, float p1, float p2) {
  Rangefunc func;
  switch ((rtp)) {
    case 1: {
      func = [=](Rvec p, Rvec d, Rvec op) { return Range::circle(p, p1, op); };
      break;
    }
    case 2: {
      func = [=](Rvec p, Rvec d, Rvec op) {
        return Range::sector(p, d, p1, p2, op);
      };
      break;
    }
    case 3: {
      func = [=](Rvec p, Rvec d, Rvec op) {
        return Range::rectangle(p, d, p1, p2, op);
      };
      break;
    }
    default: {
      break;
    }
  }
  return func;
}
