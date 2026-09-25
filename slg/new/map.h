#ifndef __MAP_H__
#define __MAP_H__

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
using std::function;
using std::string;
using std::vector;

#include "parallel_hashmap/phmap.h"
#define hash_map phmap::flat_hash_map
#define hash_set phmap::flat_hash_set

struct Pos {
  int16_t x_, y_;
  bool operator==(Pos rhs) const {
    if (x_ != rhs.x_) return false;
    return y_ == rhs.y_;
  }
  size_t operator()(Pos p) const {
    int v = p.x_ << 16 | p.y_;
    return std::hash<int>()(v);
  }
};

struct Entity {
  int64_t id_;
  Pos bl_, tr_;
};

struct March {
  int64_t id_;
  Pos cur_, end_;
};

struct Vision {
  int64_t id_;
  Pos bl_, tr_;
  vector<int64_t> march_;
  Pos center() {
    Pos c;
    c.x_ = (bl_.x_ + tr_.x_) / 2;
    c.y_ = (bl_.y_ + tr_.y_) / 2;
    return c;
  }
};

struct Map {
  static constexpr int64_t OBSTACLE_ID = -9999999;
  int16_t len_, wid_;

  hash_map<int64_t, Entity> entity_;
  hash_map<Pos, int64_t, Pos> pos_eid_;

  hash_map<int64_t, March> march_;
  hash_map<int64_t, Vision> vision_;
  hash_map<Pos, hash_set<int64_t>, Pos> pos_vids_;

  Map(int16_t len, int16_t wid);

  bool check_pos(Pos p) {
    if (p.x_ < 0) return false;
    if (p.x_ >= len_) return false;
    if (p.y_ < 0) return false;
    if (p.y_ >= wid_) return false;
    return true;
  }
  bool obstacle(Pos p) {
    if (!check_pos(p)) return true;
    return pos_eid_.find(p) != pos_eid_.end();
  }

  void seri_obstacle(string& ret);
  bool area_obstacle(Pos bl, Pos tr);

  void traversal_area(Pos bl, Pos tr, function<void(Pos)> cb);
  void add_obstalce(Pos bl, Pos tr);
  void del_obstacle(Pos bl, Pos tr);
  void add_entity(const Entity& entity);
  void del_entity(int64_t eid);
  void area_eid(Pos bl, Pos tr, vector<int64_t>& ids);

  void add_march(const March& march) { march_[march.id_] = march; }
  void del_march(int64_t id) { march_.erase(id); }
  void clear_march() { march_.clear(); }

  void set_vision(Vision& vision);
  void del_vision(int64_t id);
  void area_vid(Pos bl, Pos tr, vector<int64_t>& ids);
  bool march_in_vision(Pos cur, Pos end, Pos bl, Pos tr);
  void vision_march(int64_t vid, vector<int64_t>& ret);
  void vision_diff(int64_t vid, vector<int64_t>& adds, vector<int64_t>& dels);
};

#endif