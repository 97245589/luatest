#ifndef __JPS_H__
#define __JPS_H__

#include <cmath>
#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <functional>
#include <set>
#include <string>
#include <vector>

using std::function;
using std::multiset;
using std::string;
using std::vector;
#define hashtable __gnu_pbds::cc_hash_table
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
    int16_t u_, d_, l_, r_;
  };

  int16_t len_, wid_;
  vector<vector<bool>> block_;
  vector<vector<Pcache>> jpcache_;

  bool quick_;
  Pos start_, end_;
  vector<Pos> ret_;
  hashtable<Pos, Pos, Pos> pre_;
  multiset<State> openlist_;
  hashtable<Pos, __gnu_pbds::null_type, Pos> closelist_;

  Jps(int16_t len, int16_t wid);
  bool inarea(Pos p) {
    return p.x_ >= 0 && p.x_ <= len_ - 1 && p.y_ >= 0 && p.y_ <= wid_ - 1;
  }
  bool isblock(Pos p) {
    if (!inarea(p)) return true;
    return block_[p.x_][p.y_] == true;
  }
  static int cost(Pos p1, Pos p2) {
    int16_t dx = p1.x_ - p2.x_;
    int16_t dy = p1.y_ - p2.y_;
    return sqrt(dx * dx + dy * dy) * 100;
  };
  void setblock(int16_t x, int16_t y, bool v) {
    if (!inarea({x, y})) return;
    block_[x][y] = v;
  }
  void reset();

  void jpcache();
  void linecache(Pos p, Pos d);
  void setcache(Pos p, Pos d, int16_t v) {
    auto& pcache = jpcache_[p.x_][p.y_];
    if (d.x_ == 0 && d.y_ == 1) pcache.u_ = v;
    if (d.x_ == 0 && d.y_ == -1) pcache.d_ = v;
    if (d.x_ == -1 && d.y_ == 0) pcache.l_ = v;
    if (d.x_ == 1 && d.y_ == 0) pcache.r_ = v;
  }
  int16_t getcache(Pos p, Pos d) {
    auto& pcache = jpcache_[p.x_][p.y_];
    if (d.x_ == 0 && d.y_ == 1) return pcache.u_;
    if (d.x_ == 0 && d.y_ == -1) return pcache.d_;
    if (d.x_ == -1 && d.y_ == 0) return pcache.l_;
    if (d.x_ == 1 && d.y_ == 0) return pcache.r_;
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