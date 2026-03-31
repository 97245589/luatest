#ifndef __WORLD_H__
#define __WORLD_H__

#include <cstdint>
#include <functional>
#include <set>
#include <vector>
using std::function;
using std::set;
using std::vector;

#include "astar.h"
#include "common.h"
#include "range.h"

struct World {
  using Pos = Common::Pos;
  struct Actor {
    float x_, y_;
    int16_t dx_, dy_;
    int8_t camp_;
  };
  struct Search {
    int64_t id_;
    bool samecamp_;
    int8_t rtp_;
    float p1_, p2_;
  };
  struct Timerele {
    int64_t id_;
    int64_t buffid_;
    int64_t endtm_;
    bool operator<(const Timerele& rhs) const {
      if (endtm_ != rhs.endtm_) return endtm_ < rhs.endtm_;
      if (buffid_ != rhs.buffid_) return buffid_ < rhs.buffid_;
      return id_ < rhs.id_;
    }
  };
  using uset = hashtable<int64_t, __gnu_pbds::null_type>;

  int16_t len_, wid_;
  hashtable<int64_t, Actor> actor_;
  hashtable<Pos, uset, Pos> aoi_;
  hashtable<Pos, int8_t, Pos> block_;
  set<Timerele> timer_;
  Astar astar_;

  static constexpr int8_t AOILEN = 10;
  static constexpr int16_t AOIMAX = 200;
  World(int16_t len, int16_t wid);

  inline bool inarea(Pos);
  bool isblock(Pos);
  void setblock(Pos, int8_t v);

  void correctpos(Actor&);
  void addaoi(int64_t, const Actor&);
  void delaoi(int64_t, const Actor&);
  void addactor(int64_t id, Actor& actor);
  void delactor(int64_t id);
  void setpos(int64_t id, float x, float y, int16_t dx, int16_t dy,
              vector<int64_t>& adds, vector<int64_t>& dels);
  void aoidiff(int64_t id, Pos bp, Pos np, vector<int64_t>& adds,
               vector<int64_t>& dels);
  void traversal_area(Pos, function<void(Pos, uset&)>);
  void areaids(int64_t, vector<int64_t>&);
  void search(const Search&, vector<int64_t>&);
  using Rvec = Range::Vec;
  using Rangefunc = function<bool(Rvec p, Rvec d, Rvec op)>;
  Rangefunc range_func(int8_t rtp, float p1, float p2);
};

#endif