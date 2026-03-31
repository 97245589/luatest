#ifndef __AOI_H__
#define __AOI_H__

#include <functional>

#include "common.h"
#include "range.h"
using std::function;

struct Actor;
struct World;

struct Search {
  int64_t id_;
  bool samecamp_;
  int8_t rtp_;
  float p1_, p2_;
};

struct Aoi {
  static constexpr int8_t AOILEN = 10;
  static constexpr int16_t AOIMAX = 200;
  using uset = hashtable<int64_t, __gnu_pbds::null_type>;

  World& world_;
  hashtable<Pos, uset, Pos> aoi_;
  vector<Pos> dirs_;

  Aoi(World& w);

  void add(int64_t, const Actor&);
  void del(int64_t, const Actor&);
  void diff(int64_t id, Pos bp, Pos np, vector<int64_t>& adds,
            vector<int64_t>& dels);
  void traversal_area(Pos, function<void(Pos, uset&)>);
  void aoi_ids(int64_t, vector<int64_t>&);
  void search(const Search&, vector<int64_t>&);
  using Rvec = Range::Vec;
  using Rangefunc = function<bool(Rvec p, Rvec d, Rvec op)>;
  Rangefunc range_func(int8_t rtp, float p1, float p2);
};

#endif