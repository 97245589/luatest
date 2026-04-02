#include "world.h"

#include <algorithm>
#include <iostream>
using namespace std;

World::World(int16_t len, int16_t wid) : len_(len), wid_(wid) {
  block_ = vector<vector<int64_t>>(len, vector<int64_t>(wid, 0));
}

bool World::checkpos(Pos p) {
  if (p.x_ < 0) return false;
  if (p.x_ > len_ - 1) return false;
  if (p.y_ < 0) return false;
  if (p.y_ > wid_ - 1) return false;
  return true;
}

bool World::isblock(Pos p) {
  if (!checkpos(p)) return false;
  return block_[p.x_][p.y_] != 0;
}

void World::correctpos(Pos& p) {
  if (p.x_ < 0) p.x_ = 0;
  if (p.x_ > len_ - 1) p.x_ = len_ - 1;
  if (p.y_ < 0) p.y_ = 0;
  if (p.y_ > wid_ - 1) p.y_ = wid_ - 1;
}

void World::traversal_area(Pos bl, Pos tr, function<void(Pos)> func) {
  correctpos(bl);
  correctpos(tr);
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos p{x, y};
      func(p);
    }
  }
}

bool World::areablock(Pos bl, Pos tr) {
  if (isblock(bl)) return true;
  if (isblock(tr)) return true;
  bool b = false;
  traversal_area(bl, tr, [&](Pos p) {
    if (isblock(p)) b = true;
  });
  return b;
}

void World::setblock(Pos bl, Pos tr, int64_t v) {
  traversal_area(bl, tr, [&](Pos p) { block_[p.x_][p.y_] = v; });
}

void World::areaids(Pos bl, Pos tr, vector<int64_t>& ret) {
  ret.reserve(32);
  correctpos(bl);
  correctpos(tr);
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      int64_t id = block_[x][y];
      if (id > 0) ret.push_back(id);
    }
  }
  sort(ret.begin(), ret.end());
  ret.erase(unique(ret.begin(), ret.end()), ret.end());
}

bool World::inview(Pos n, Pos e, Pos bl, Pos tr) {
  auto inarea = [](Pos p, Pos bl, Pos tr) {
    if (p.x_ < bl.x_) return false;
    if (p.x_ > tr.x_) return false;
    if (p.y_ < bl.y_) return false;
    if (p.y_ > tr.y_) return false;
    return true;
  };
  auto cross = [](Pos a, Pos b, Pos c) {
    Pos vab;
    vab.x_ = b.x_ - a.x_;
    vab.y_ = b.y_ - a.y_;
    Pos vac;
    vac.x_ = c.x_ - a.x_;
    vac.y_ = c.y_ - a.y_;
    float v = vab.x_ * vac.y_ - vac.x_ * vab.y_;
    return v;
  };
  auto intersect = [=](Pos a, Pos b, Pos c, Pos d) {
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
  };

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
    auto& btroop = view.troop_;
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