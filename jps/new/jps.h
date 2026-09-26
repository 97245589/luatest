#ifndef __JPS_H__
#define __JPS_H__

#include <cmath>
#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <vector>

using std::function;
using std::multiset;
using std::string;
using std::vector;

#include "parallel_hashmap/phmap.h"
#define hash_map phmap::flat_hash_map
#define hash_set phmap::flat_hash_set
struct Jps {
  struct Pos {
    int16_t x_, y_;
    bool operator==(const Pos& rhs) const {
      return x_ == rhs.x_ && y_ == rhs.y_;
    }
    size_t operator()(const Pos& p) const {
      int v = p.x_ << 16 | p.y_;
      return std::hash<int>()(v);
    }
  };
  struct State {
    int16_t x_, y_;
    int32_t cost_, weigh_;
    int16_t dx_, dy_;
    bool operator<(const State& rhs) const { return weigh_ < rhs.weigh_; }
  };
  struct Pcache {
    int16_t up_, down_, left_, right_;
  };

  int16_t len_, wid_;
  vector<vector<bool>> obstacle_;
  vector<vector<Pcache>> jpcache_;

  bool quick_;
  Pos start_, end_;
  vector<Pos> ret_;
  hash_map<Pos, Pos, Pos> pre_;
  multiset<State> openlist_;
  hash_set<Pos, Pos> closelist_;

  Jps(int16_t len, int16_t wid);
  bool inarea(Pos p) {
    return p.x_ >= 0 && p.x_ <= len_ - 1 && p.y_ >= 0 && p.y_ <= wid_ - 1;
  }
  bool obstacle(Pos p) {
    if (!inarea(p)) return true;
    return obstacle_[p.x_][p.y_] == true;
  }
  static int cost(Pos p1, Pos p2) {
    int16_t dx = p1.x_ - p2.x_;
    int16_t dy = p1.y_ - p2.y_;
    return sqrt(dx * dx + dy * dy) * 100;
  };
  void set_obstacle(int16_t x, int16_t y, bool v) {
    if (!inarea({x, y})) return;
    obstacle_[x][y] = v;
  }
  void reset();

  void jpcache();
  void linecache(Pos p, Pos d);
  void setcache(Pos p, Pos d, int16_t v) {
    auto& pcache = jpcache_[p.x_][p.y_];
    if (d == Pos{0, 1}) pcache.up_ = v;
    if (d == Pos{0, -1}) pcache.down_ = v;
    if (d == Pos{-1, 0}) pcache.left_ = v;
    if (d == Pos{1, 0}) pcache.right_ = v;
  }
  int16_t getcache(Pos p, Pos d) {
    auto& pcache = jpcache_[p.x_][p.y_];
    if (d == Pos{0, 1}) return pcache.up_;
    if (d == Pos{0, -1}) return pcache.down_;
    if (d == Pos{-1, 0}) return pcache.left_;
    if (d == Pos{1, 0}) return pcache.right_;
    return 0;
  }
  string dumpcache(Pos d);

  void genret();
  void fneig(Pos p, Pos d, function<void(Pos, Pos)> cb);
  bool addfn(State s, Pos d);
  bool addjp(State s, Pos pre);
  bool checkend(Pos p1, Pos p2);
  bool step(State s, Pos dir);
  void find();
};

#endif