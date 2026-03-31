#ifndef __JPS_H__
#define __JPS_H__

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::array;
using std::function;
using std::priority_queue;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

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
    bool operator<(const State& rhs) const { return weigh_ > rhs.weigh_; }
  };

  int16_t len_, wid_;
  vector<vector<int8_t>> block_;
  vector<vector<array<int16_t, 4>>> jpcache_;

  bool quick_;
  Pos start_, end_;
  vector<Pos> ret_;
  unordered_map<Pos, Pos, Pos> pre_;
  priority_queue<State> openlist_;
  unordered_set<Pos, Pos> closelist_;

  Jps(int16_t len, int16_t wid);
  bool inarea(Pos p) {
    if (p.x_ < 0 || p.x_ > len_ - 1) return false;
    if (p.y_ < 0 || p.y_ > wid_ - 1) return false;
    return true;
  }
  bool isblock(Pos p) {
    if (!inarea(p)) return true;
    return block_[p.x_][p.y_] != 0;
  }
  static int cost(Pos p1, Pos p2) {
    int16_t dx = p1.x_ - p2.x_;
    int16_t dy = p1.y_ - p2.y_;
    return sqrt(dx * dx + dy * dy) * 100;
  };
  void setblock(int16_t x, int16_t y, int8_t v) {
    if (!inarea({x, y})) return;
    block_[x][y] = v;
  }
  void reset();

  void jpcache();
  void linecache(Pos p, Pos d);
  void setcache(Pos p, Pos d, int16_t v);
  int16_t getcache(Pos p, Pos d);
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