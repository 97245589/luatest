#include <cmath>
#include <set>
using namespace std;

#include "world.h"

static constexpr int LWEIGH = 100;
static constexpr int HWEIGH = 141;
struct Dircost {
  int8_t x_, y_;
  int16_t cost_;
};
static const vector<Dircost> DIRECT_COST = {
    {0, 1, LWEIGH}, {0, -1, LWEIGH}, {1, 0, LWEIGH},  {-1, 0, LWEIGH},
    {1, 1, HWEIGH}, {-1, 1, HWEIGH}, {1, -1, HWEIGH}, {-1, -1, HWEIGH}};

inline static int cost(Pos s, Pos e) {
  int16_t dx = s.x_ - e.x_;
  int16_t dy = s.y_ - e.y_;
  return 100 * sqrt(dx * dx + dy * dy);
}

bool Astar::isblock(Pos p) { return world_.isblock(p); }

void Astar::find(Pos s, Pos e, vector<Pos>& ret, bool quick) {
  if (isblock(s) || isblock(e)) return;
  if (s == e) return;
  hashtable<Pos, __gnu_pbds::null_type, Pos> closelist;
  hashtable<Pos, Pos, Pos> pres;
  multiset<State> openlist;
  openlist.insert({s.x_, s.y_, 0, cost(s, e)});

  while (!openlist.empty()) {
    auto oit = openlist.begin();
    State s = *oit;
    openlist.erase(oit);
    Pos p{.x_ = s.x_, .y_ = s.y_};
    if (p == e) {
      ret.push_back(p);
      while (pres.end() != pres.find(p)) {
        auto it = pres.find(p);
        auto& pre = it->second;
        p = {.x_ = pre.x_, .y_ = pre.y_};
        int rsize = ret.size();
        if (rsize >= 2) {
          Pos pp = ret[rsize - 1];
          Pos ppp = ret[rsize - 2];
          int16_t dx1 = pp.x_ - p.x_;
          int16_t dy1 = pp.y_ - p.y_;
          int16_t dx2 = ppp.x_ - p.x_;
          int16_t dy2 = ppp.y_ - p.y_;
          if (dx1 * dy2 == dy1 * dx2) {
            ret[rsize - 1] = p;
          } else {
            ret.push_back(p);
          }
        } else {
          ret.push_back(p);
        }
      }
      return;
    }
    if (closelist.find(p) != closelist.end()) continue;
    closelist.insert(p);
    for (const Dircost& d : DIRECT_COST) {
      State s1;
      s1.x_ = s.x_ + d.x_;
      s1.y_ = s.y_ + d.y_;
      s1.cost_ = s.cost_ + d.cost_;
      Pos p1{s1.x_, s1.y_};
      if (isblock(p1)) continue;
      if (closelist.find(p1) != closelist.end()) continue;
      if (p1 == e) {
        s1.weigh_ = 0;
      } else {
        if (quick) {
          s1.weigh_ = cost(p1, e);
        } else {
          s1.weigh_ = s1.cost_ + cost(p1, e);
        }
      }
      if (auto it = pres.find(p1); it == pres.end()) {
        openlist.insert(s1);
        pres[p1] = p;
      }
    }
  }
}
