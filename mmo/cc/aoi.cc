#include "aoi.h"

#include <algorithm>
#include <iostream>

#include "world.h"
using namespace std;

Aoi::Aoi(World& w) : world_(w) {
  for (int8_t i = -1; i <= 1; ++i) {
    for (int8_t j = -1; j <= 1; ++j) {
      dirs_.push_back({i, j});
    }
  }
  sort(dirs_.begin(), dirs_.end(), [](Pos lhs, Pos rhs) {
    int8_t lx = lhs.x_;
    int8_t ly = lhs.y_;
    int8_t rx = rhs.x_;
    int8_t ry = rhs.y_;
    return lx * lx + ly * ly < rx * rx + ry * ry;
  });
}

void Aoi::add(int64_t id, const Actor& actor) {
  Pos g;
  g.x_ = actor.x_ / AOILEN;
  g.y_ = actor.y_ / AOILEN;
  aoi_[g].insert(id);
}

void Aoi::del(int64_t id, const Actor& actor) {
  Pos g;
  g.x_ = actor.x_ / AOILEN;
  g.y_ = actor.y_ / AOILEN;
  auto it = aoi_.find(g);
  if (it == aoi_.end()) return;
  auto& ids = it->second;
  ids.erase(id);
  if (ids.empty()) aoi_.erase(g);
}

void Aoi::traversal_area(Pos p, function<void(Pos, uset&)> cb) {
  for (Pos d : dirs_) {
    Pos ng;
    ng.x_ = p.x_ / AOILEN + d.x_;
    ng.y_ = p.y_ / AOILEN + d.y_;
    auto it = aoi_.find(ng);
    if (it == aoi_.end()) continue;
    auto& ids = it->second;
    cb(ng, ids);
  }
}

void Aoi::diff(int64_t aid, Pos bp, Pos np, vector<int64_t>& adds,
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

void Aoi::aoi_ids(int64_t id, vector<int64_t>& ret) {
  auto& actor_ = world_.actor_;
  auto it = actor_.find(id);
  if (it == actor_.end()) return;
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

struct Dis {
  int64_t id_;
  float dis_;
};
void Aoi::search(const Search& info, vector<int64_t>& ret) {
  auto& actor_ = world_.actor_;
  ret.reserve(32);
  int64_t id = info.id_;
  int16_t num = info.num_;
  if (num <= 0) return;
  bool samecamp = info.samecamp_;
  auto ait = actor_.find(id);
  if (ait == actor_.end()) return;
  Rangefunc rangefunc = range_func(info.rtp_, info.p1_, info.p2_);
  if (!rangefunc) return;
  vector<int64_t> ids;
  aoi_ids(id, ids);
  Actor& actor = ait->second;
  Rvec p{.x_ = actor.x_, .y_ = actor.y_};
  Rvec d;
  d.x_ = actor.dx_;
  d.y_ = actor.dy_;
  vector<Dis> dis;
  for (int64_t oid : ids) {
    auto oit = actor_.find(oid);
    if (oit == actor_.end()) continue;
    Actor& oactor = oit->second;
    if (samecamp && actor.camp_ != oactor.camp_) continue;
    if (!samecamp && actor.camp_ == oactor.camp_) continue;
    Rvec op{.x_ = oactor.x_, .y_ = oactor.y_};
    if (rangefunc(p, d, op)) {
      Dis od;
      od.id_ = oid;
      float dx = oactor.x_ - actor.x_;
      float dy = oactor.y_ - actor.y_;
      od.dis_ = dx * dx + dy * dy;
      dis.push_back(od);
    }
  }
  if (dis.size() <= num) {
    for (auto v : dis) {
      ret.push_back(v.id_);
    }
  } else {
    nth_element(dis.begin(), dis.begin() + num - 1, dis.end(),
                [](Dis lhs, Dis rhs) { return lhs.dis_ < rhs.dis_; });
    for (int i = 0; i < num; ++i) {
      ret.push_back(dis[i].id_);
    }
  }
}

Aoi::Rangefunc Aoi::range_func(int8_t rtp, float p1, float p2) {
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
