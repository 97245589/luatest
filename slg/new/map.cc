#include <iostream>
using namespace std;

#include "map.h"

Map::Map(int16_t len, int16_t wid) {
  len_ = len;
  wid_ = wid;
}

bool Map::area_obstacle(Pos bl, Pos tr) {
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos p{.x_ = x, .y_ = y};
      if (obstacle(p)) return true;
    }
  }
  return false;
}

void Map::seri_obstacle(string& ret) {
  ret.reserve(1024);
  for (auto& [p, eid] : pos_eid_) {
    ret.append((char*)&p, sizeof(p));
  }
}

void Map::traversal_area(Pos bl, Pos tr, function<void(Pos)> cb) {
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos p{.x_ = x, .y_ = y};
      if (!check_pos(p)) continue;
      cb(p);
    }
  }
}

void Map::add_obstalce(Pos bl, Pos tr) {
  traversal_area(bl, tr, [&](Pos p) { pos_eid_[p] = OBSTACLE_ID; });
}

void Map::del_obstacle(Pos bl, Pos tr) {
  traversal_area(bl, tr, [&](Pos p) { pos_eid_.erase(p); });
}

void Map::add_entity(const Entity& entity) {
  int64_t eid = entity.id_;
  entity_[eid] = entity;
  traversal_area(entity.bl_, entity.tr_, [&](Pos p) { pos_eid_[p] = eid; });
}

void Map::del_entity(int64_t id) {
  auto it = entity_.find(id);
  if (it == entity_.end()) return;
  Entity& entity = it->second;
  traversal_area(entity.bl_, entity.tr_, [&](Pos p) { pos_eid_.erase(p); });
  entity_.erase(it);
}

void Map::area_eid(Pos bl, Pos tr, vector<int64_t>& ids) {
  ids.reserve(16);
  hash_set<int64_t> set_;
  traversal_area(bl, tr, [&](Pos p) {
    auto it = pos_eid_.find(p);
    if (it == pos_eid_.end()) return;
    int64_t id = it->second;
    if (id == OBSTACLE_ID) return;
    set_.insert(id);
  });
  ids.insert(ids.end(), set_.begin(), set_.end());
}

void Map::set_vision(Vision& vision) {
  int64_t vid = vision.id_;
  if (auto it = vision_.find(vid); it != vision_.end()) {
    vision.march_ = std::move(it->second.march_);
    del_vision(vid);
  }
  vision_[vid] = std::move(vision);
  Pos c = vision.center();
  pos_vids_[c].insert(vid);
}

void Map::del_vision(int64_t id) {
  auto it = vision_.find(id);
  if (it == vision_.end()) return;
  Vision& v = it->second;
  Pos c = v.center();
  if (auto pit = pos_vids_.find(c); pit != pos_vids_.end()) {
    auto& set_ = pit->second;
    set_.erase(id);
    if (set_.empty()) pos_vids_.erase(pit);
  }
  vision_.erase(it);
}

void Map::area_vid(Pos bl, Pos tr, vector<int64_t>& ids) {
  ids.reserve(16);
  for (int16_t x = bl.x_; x <= tr.x_; ++x) {
    for (int16_t y = bl.y_; y <= tr.y_; ++y) {
      Pos c{.x_ = x, .y_ = y};
      auto it = pos_vids_.find(c);
      if (it == pos_vids_.end()) continue;
      auto& set_ = it->second;
      ids.insert(ids.end(), set_.begin(), set_.end());
    }
  }
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
bool Map::march_in_vision(Pos cur, Pos end, Pos bl, Pos tr) {
  if (inarea(cur, bl, tr) || inarea(end, bl, tr)) return true;
  Pos p1{bl.x_, bl.y_};
  Pos p2{bl.x_, tr.y_};
  Pos p3{tr.x_, tr.y_};
  Pos p4{tr.x_, bl.y_};
  return intersect(cur, end, p1, p2) || intersect(cur, end, p2, p3) ||
         intersect(cur, end, p3, p4) || intersect(cur, end, p4, p1);
}

void Map::vision_march(int64_t vid, vector<int64_t>& ret) {
  auto it = vision_.find(vid);
  if (it == vision_.end()) return;
  Vision& v = it->second;
  ret.reserve(32);
  for (auto& [mid, march] : march_) {
    if (march_in_vision(march.cur_, march.end_, v.bl_, v.tr_)) {
      ret.push_back(mid);
    }
  }
}

void Map::vision_diff(int64_t vid, vector<int64_t>& adds,
                      vector<int64_t>& dels) {
  auto it = vision_.find(vid);
  if (it == vision_.end()) return;
  adds.reserve(32);
  dels.reserve(32);
  Vision& vis = it->second;
  vector<int64_t> new_;
  vision_march(vid, new_);

  vector<int64_t>& old = vis.march_;
  sort(new_.begin(), new_.end());
  set_difference(old.begin(), old.end(), new_.begin(), new_.end(),
                 back_inserter(dels));
  set_difference(new_.begin(), new_.end(), old.begin(), old.end(),
                 back_inserter(adds));
  vis.march_ = std::move(new_);
}