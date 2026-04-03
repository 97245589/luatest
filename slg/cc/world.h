#ifndef __WORLD_H__
#define __WORLD_H__

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <functional>
#include <vector>
using std::function;
using std::vector;
#define hashtable __gnu_pbds::cc_hash_table

struct Pos {
  int16_t x_, y_;
  bool operator==(Pos rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
  size_t operator()(Pos p) const {
    int v = p.x_ << 16 | p.y_;
    return std::hash<int>()(v);
  }
};

struct Troop {
  Pos n_, e_;
};

struct View {
  Pos bl_, tr_;
  vector<int64_t> troop_;
};

struct Entity {
  Pos bl_, tr_;
};

struct World {
  int16_t len_, wid_;
  hashtable<int64_t, Entity> entity_;
  hashtable<Pos, int64_t, Pos> pos_entity_;
  vector<vector<bool>> block_;
  hashtable<int64_t, Troop> troop_;
  hashtable<int64_t, View> view_;

  World(int16_t len, int16_t wid);

  bool checkpos(Pos p) {
    return p.x_ >= 0 && p.x_ <= len_ - 1 && p.y_ >= 0 && p.y_ <= wid_ - 1;
  }
  bool isblock(Pos p) {
    if (!checkpos(p)) return true;
    return block_[p.x_][p.y_] == true;
  }
  void correctpos(Pos& p);
  bool haveblock(Pos bl, Pos tr);
  void setblock(Pos p, bool v) {
    if (!checkpos(p)) return;
    block_[p.x_][p.y_] = v;
  }
  void addentity(int64_t, const Entity&);
  void delentity(int64_t);
  void areaids(Pos bl, Pos tr, vector<int64_t>& ret);

  bool inview(Pos n, Pos e, Pos bl, Pos tr);
  void troop_view(function<void(int64_t, vector<int64_t>&, vector<int64_t>&)>);
};

#endif