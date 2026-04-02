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
};

struct Troop {
  Pos n_, e_;
};

struct View {
  Pos bl_, tr_;
  vector<int64_t> troop_;
};

struct World {
  int16_t len_, wid_;
  vector<vector<int64_t>> block_;
  hashtable<int64_t, Troop> troop_;
  hashtable<int64_t, View> view_;

  World(int16_t len, int16_t wid);

  inline bool checkpos(Pos);
  inline bool isblock(Pos);
  inline void correctpos(Pos& p);
  void traversal_area(Pos bl, Pos tr, function<void(Pos)> func);
  void setblock(Pos bl, Pos tr, int64_t v);
  bool areablock(Pos bl, Pos tr);
  void areaids(Pos bl, Pos tr, vector<int64_t>& ret);

  bool inview(Pos n, Pos e, Pos bl, Pos tr);
  void troop_view(function<void(int64_t, vector<int64_t>&, vector<int64_t>&)>);
};

#endif