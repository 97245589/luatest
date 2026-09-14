#ifndef __MAP_H__
#define __MAP_H__

#include <functional>
#include <vector>
using std::function;
using std::vector;

#include "astar.h"
#include "common.h"

struct Entity {
  int64_t id_;
  float x_, y_;
  int16_t dx_, dy_;
  int8_t atk_type_;
};

struct Search {
  int64_t id_;
  int num_;
  int8_t atk_type_;  // -1=all 0=diff_atktype 1=same_atktype
  int8_t range_tp_;
  float p1_, p2_;
};

struct Map {
  static constexpr int16_t search_max_ = 3;
  static constexpr int16_t cell_len_ = 4;
  int16_t len_;
  int16_t wid_;
  Astar astar_;

  Map(int16_t len, int16_t wid);

  using IDSET = hash_table<int64_t, __gnu_pbds::null_type>;
  vector<Pos> search_order_;
  hash_table<int64_t, Entity> entities_;
  hash_table<Pos, IDSET, Pos> cell_entities_;
  hash_table<Pos, __gnu_pbds::null_type, Pos> obstacle_;

  bool in_area(Pos p) {
    if (p.x_ < 0) return false;
    if (p.x_ > len_ - 1) return false;
    if (p.y_ < 0) return false;
    if (p.y_ > wid_ - 1) return false;
    return true;
  }
  bool impassable(Pos p) {
    if (!in_area(p)) return true;
    return obstacle_.find(p) != obstacle_.end();
  }
  void add_obstacle(Pos p) {
    if (!in_area(p)) return;
    obstacle_.insert(p);
  }
  void del_obstacle(Pos p) { obstacle_.erase(p); }

  void add_entity(Entity& entity);
  void del_entity(int64_t id);
  void update_pos(int64_t id, float x, float y, int16_t dx, int16_t dy);
  void add_cell(Entity& entity);
  void del_cell(Entity& entity);

  void diff_aoi(Pos ocell, Pos ncell, vector<int64_t>& add,
                vector<int64_t>& del);
  void get_aoi(Pos cell, function<void(Pos c, IDSET& ids)> cb);
  void aoi_search(Search info, vector<int64_t>& ret);
};

#endif