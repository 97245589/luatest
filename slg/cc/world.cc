#include "world.h"

#include <algorithm>
#include <iostream>
using namespace std;

World::World(int16_t len, int16_t wid) : len_(len), wid_(wid) {
  block_ = vector<vector<bool>>(len, vector<bool>(wid, false));
}

void World::correctpos(Pos& p) {
  if (p.x_ < 0) p.x_ = 0;
  if (p.x_ > len_ - 1) p.x_ = len_ - 1;
  if (p.y_ < 0) p.y_ = 0;
  if (p.y_ > wid_ - 1) p.y_ = wid_ - 1;
}

bool World::haveblock(Pos bl, Pos tr) {
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      if (isblock({x, y})) return true;
    }
  }
  return false;
}

void World::addentity(int64_t id, const Entity& entity) {
  auto it = entity_.find(id);
  if (it != entity_.end()) return;
  Pos bl = entity.bl_;
  Pos tr = entity.tr_;
  if (haveblock(bl, tr)) return;
  entity_.insert({id, entity});
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos p{x, y};
      pos_entity_[p] = id;
      setblock(p, true);
    }
  }
}

void World::delentity(int64_t id) {
  auto it = entity_.find(id);
  if (it == entity_.end()) return;
  Entity& entity = it->second;
  Pos bl = entity.bl_;
  Pos tr = entity.tr_;
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos p{x, y};
      pos_entity_.erase(p);
      setblock(p, false);
    }
  }
  entity_.erase(id);
}

void World::areaids(Pos bl, Pos tr, vector<int64_t>& ret) {
  ret.reserve(32);
  correctpos(bl);
  correctpos(tr);
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      auto it = pos_entity_.find({x, y});
      if (it == pos_entity_.end()) continue;
      ret.push_back(it->second);
    }
  }
  sort(ret.begin(), ret.end());
  ret.erase(unique(ret.begin(), ret.end()), ret.end());
}

inline static bool inarea(Pos p, Pos bl, Pos tr) {
  if (p.x_ < bl.x_) return false;
  if (p.x_ > tr.x_) return false;
  if (p.y_ < bl.y_) return false;
  if (p.y_ > tr.y_) return false;
  return true;
}
inline static float cross(Pos a, Pos b, Pos c) {
  Pos vab;
  vab.x_ = b.x_ - a.x_;
  vab.y_ = b.y_ - a.y_;
  Pos vac;
  vac.x_ = c.x_ - a.x_;
  vac.y_ = c.y_ - a.y_;
  return vab.x_ * vac.y_ - vac.x_ * vab.y_;
}
inline static bool intersect(Pos a, Pos b, Pos c, Pos d) {
  if (std::max(a.x_, b.x_) < std::min(c.x_, d.x_) ||
      std::max(c.x_, d.x_) < std::min(a.x_, b.x_) ||
      std::max(a.y_, b.y_) < std::min(c.y_, d.y_) ||
      std::max(c.y_, d.y_) < std::min(a.y_, b.y_)) {
    return false;
  }
  float c1 = cross(a, b, c);
  float c2 = cross(a, b, d);
  float c3 = cross(c, d, a);
  float c4 = cross(c, d, b);
  return (c1 * c2 <= 0) && (c3 * c4 <= 0);
}
bool World::inview(Pos n, Pos e, Pos bl, Pos tr) {
  if (inarea(n, bl, tr) || inarea(e, bl, tr)) return true;
  Pos p1{bl.x_, bl.y_};
  Pos p2{bl.x_, tr.y_};
  Pos p3{tr.x_, tr.y_};
  Pos p4{tr.x_, bl.y_};
  return intersect(n, e, p1, p2) || intersect(n, e, p2, p3) ||
         intersect(n, e, p3, p4) || intersect(n, e, p4, p1);
}

void World::troop_view(
    function<void(int64_t, vector<int64_t>&, vector<int64_t>&)> cb) {
  vector<int64_t> adds;
  vector<int64_t> dels;
  adds.reserve(32);
  dels.reserve(32);
  for (auto& [vid, view] : view_) {
    adds.clear();
    dels.clear();
    Pos bl = view.bl_;
    Pos tr = view.tr_;
    vector<int64_t>& btroop = view.troop_;
    vector<int64_t> ntroop;
    ntroop.reserve(32);
    for (auto& [id, troop] : troop_) {
      if (inview(troop.n_, troop.e_, bl, tr)) {
        ntroop.push_back(id);
      }
    }
    sort(ntroop.begin(), ntroop.end());
    set_difference(btroop.begin(), btroop.end(), ntroop.begin(), ntroop.end(),
                   back_inserter(dels));
    set_difference(ntroop.begin(), ntroop.end(), btroop.begin(), btroop.end(),
                   back_inserter(adds));
    view.troop_ = std::move(ntroop);
    if (adds.empty() && dels.empty()) continue;
    cb(vid, adds, dels);
  }
}