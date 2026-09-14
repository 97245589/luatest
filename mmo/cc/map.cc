#include <algorithm>
#include <iostream>
using namespace std;

#include "map.h"
#include "range.h"

Map::Map(int16_t len, int16_t wid) : astar_(*this) {
  len_ = len;
  wid_ = wid;
  int16_t max = search_max_;
  for (int16_t x = -max; x <= max; ++x) {
    for (int16_t y = -max; y <= max; ++y) {
      search_order_.push_back({x, y});
    }
  }
  sort(search_order_.begin(), search_order_.end());
}

void Map::add_entity(Entity& entity) {
  int64_t id = entity.id_;
  if (entities_.find(id) != entities_.end()) return;
  Pos p;
  if (impassable(p)) return;
  entities_[id] = entity;
  add_cell(entity);
}

void Map::update_pos(int64_t id, float x, float y, int16_t dx, int16_t dy) {
  auto it = entities_.find(id);
  if (it == entities_.end()) return;
  Entity& entity = it->second;
  del_cell(entity);
  entity.x_ = x;
  entity.y_ = y;
  entity.dx_ = dx;
  entity.dy_ = dy;
  add_cell(entity);
}

void Map::del_entity(int64_t id) {
  auto it = entities_.find(id);
  if (it == entities_.end()) return;
  Entity& entity = it->second;
  del_cell(entity);
  entities_.erase(id);
}

void Map::add_cell(Entity& entity) {
  Pos p;
  p.x_ = entity.x_ / cell_len_;
  p.y_ = entity.y_ / cell_len_;
  cell_entities_[p].insert(entity.id_);
}

void Map::del_cell(Entity& entity) {
  int64_t id = entity.id_;
  Pos p;
  p.x_ = entity.x_ / cell_len_;
  p.y_ = entity.y_ / cell_len_;
  auto it = cell_entities_.find(p);
  if (it == cell_entities_.end()) return;
  auto& entities = it->second;
  entities.erase(id);
  if (entities.empty()) cell_entities_.erase(p);
}

void Map::diff_aoi(Pos ocell, Pos ncell, vector<int64_t>& add,
                   vector<int64_t>& del) {
  if (ocell == ncell) return;
  add.reserve(64);
  del.reserve(64);

  int16_t ox = ocell.x_;
  int16_t oy = ocell.y_;
  int16_t nx = ncell.x_;
  int16_t ny = ncell.y_;
  int16_t max = search_max_;

  get_aoi(ocell, [&](Pos cell, IDSET& ids) {
    if (cell.x_ >= nx - max && cell.x_ <= nx + max && cell.y_ >= ny - max &&
        cell.y_ <= ny + max)
      return;
    del.insert(del.end(), ids.begin(), ids.end());
  });
  get_aoi(ncell, [&](Pos cell, IDSET& ids) {
    if (cell.x_ >= ox - max && cell.x_ <= ox + max && cell.y_ >= ny - max &&
        cell.y_ <= ny + max)
      return;
    add.insert(add.end(), ids.begin(), ids.end());
  });
}

void Map::get_aoi(Pos cell, function<void(Pos c, IDSET& ids)> cb) {
  for (Pos ps : search_order_) {
    int16_t cx = cell.x_ + ps.x_;
    int16_t cy = cell.y_ + ps.y_;
    Pos cell{cx, cy};
    auto cit = cell_entities_.find(cell);
    if (cit == cell_entities_.end()) continue;
    cb(cell, cit->second);
  }
}

void Map::aoi_search(Search info, vector<int64_t>& ret) {
  auto range_func = [=](Range::Vec p1, Range::Vec d, Range::Vec p2) {
    switch (info.range_tp_) {
      case 1: {
        return Range::circle(p1, info.p1_, p2);
      }
      case 2: {
        return Range::sector(p1, d, info.p1_, info.p2_, p2);
      }
      case 3: {
        return Range::rectangle(p1, d, info.p1_, info.p2_, p2);
      }
    }
    return false;
  };

  ret.reserve(64);
  int64_t id = info.id_;
  int8_t atk_type = info.atk_type_;
  auto eit = entities_.find(id);
  if (eit == entities_.end()) return;
  Entity& entity = eit->second;
  Pos cell;
  cell.x_ = entity.x_ / cell_len_;
  cell.y_ = entity.y_ / cell_len_;
  Range::Vec p1, d;
  p1.x_ = entity.x_;
  p1.y_ = entity.y_;
  d.x_ = entity.dx_;
  d.y_ = entity.dy_;

  get_aoi(cell, [&](Pos cell, IDSET& ids) {
    if (ret.size() >= info.num_) return;
    for (int64_t oid : ids) {
      auto oeit = entities_.find(oid);
      if (oeit == entities_.end()) continue;
      Entity& oentity = oeit->second;
      if (atk_type == 0 && entity.atk_type_ == oentity.atk_type_) continue;
      if (atk_type == 1 && entity.atk_type_ != oentity.atk_type_) continue;
      Range::Vec p2;
      p2.x_ = oentity.x_;
      p2.y_ = oentity.y_;
      if (!range_func(p1, d, p2)) continue;
      ret.push_back(oid);
      if (ret.size() >= info.num_) return;
    }
  });
}