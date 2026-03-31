#include "jps.h"

#include <iomanip>
#include <iostream>
#include <sstream>
using namespace std;

using Pos = Jps::Pos;
static constexpr int16_t LWEI = 100;
static constexpr int16_t HWEI = 141;
static const vector<Pos> DIR = {{0, 1}, {0, -1}, {1, 0},  {-1, 0},
                                {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
static const unordered_map<Pos, int8_t, Pos> DIRIDX = {
    {{0, 1}, 0}, {{0, -1}, 1}, {{1, 0}, 2}, {{-1, 0}, 3}};

Jps::Jps(int16_t len, int16_t wid) : len_(len), wid_(wid) {
  block_ = vector<vector<int8_t>>(len, vector<int8_t>(wid, 0));
}

void Jps::reset() {
  ret_.clear();
  pre_.clear();
  closelist_.clear();
  openlist_ = {};
}

int16_t Jps::getcache(Pos p, Pos d) {
  auto it = DIRIDX.find(d);
  if (it == DIRIDX.end()) return 0;
  return jpcache_[p.x_][p.y_][it->second];
}
void Jps::setcache(Pos p, Pos d, int16_t v) {
  auto it = DIRIDX.find(d);
  if (it == DIRIDX.end()) return;
  jpcache_[p.x_][p.y_][it->second] = v;
}
void Jps::linecache(Pos p, Pos d) {
  Pos q = p;
  while (inarea(q) && isblock(q)) {
    setcache(q, d, 0);
    q.x_ += d.x_;
    q.y_ += d.y_;
  }
  if (!inarea(q)) return;

  Pos t = q;
  int16_t i = 0;
  while (true) {
    bool m = false;
    fneig(t, d, [&](Pos p1, Pos p2) { m = true; });
    if (isblock(t) || (m && i != 0)) {
      if (isblock(t)) --i;
      for (int16_t j = 0; j <= i; ++j) {
        Pos t2;
        t2.x_ = q.x_ + d.x_ * j;
        t2.y_ = q.y_ + d.y_ * j;
        setcache(t2, d, i - j);
      }
      linecache(t, d);
      return;
    } else {
      t.x_ += d.x_;
      t.y_ += d.y_;
      ++i;
    }
  }
}
void Jps::jpcache() {
  jpcache_ =
      vector<vector<array<int16_t, 4>>>(len_, vector<array<int16_t, 4>>(wid_));

  for (int16_t i = 0; i < len_; ++i) {
    Pos p{.x_ = i, .y_ = 0};
    linecache(p, {0, 1});
    Pos p1;
    p1.x_ = i;
    p1.y_ = wid_ - 1;
    linecache(p1, {0, -1});
  }
  for (int16_t j = 0; j < wid_; ++j) {
    Pos p{.x_ = 0, .y_ = j};
    linecache(p, {1, 0});
    Pos p1;
    p1.x_ = len_ - 1;
    p1.y_ = j;
    linecache(p1, {-1, 0});
  }
}
string Jps::dumpcache(Pos d) {
  ostringstream oss;
  for (int16_t y = wid_ - 1; y >= 0; --y) {
    for (int16_t x = 0; x < len_; ++x) {
      int16_t v = getcache({x, y}, d);
      oss << setw(3) << v;
    }
    oss << endl;
  }
  return oss.str();
}

void Jps::fneig(Pos p, Pos d, function<void(Pos, Pos)> cb) {
  Pos p1;
  p1.x_ = p.x_ + d.y_;
  p1.y_ = p.y_ + d.x_;
  Pos p11;
  p11.x_ = p1.x_ + d.x_;
  p11.y_ = p1.y_ + d.y_;
  if (inarea(p1) && isblock(p1) && !isblock(p11)) {
    Pos nd;
    nd.x_ = d.x_ + d.y_;
    nd.y_ = d.y_ + d.x_;
    cb(p11, nd);
  }

  Pos p2;
  p2.x_ = p.x_ - d.y_;
  p2.y_ = p.y_ - d.x_;
  Pos p21;
  p21.x_ = p2.x_ + d.x_;
  p21.y_ = p2.y_ + d.y_;
  if (inarea(p2) && isblock(p2) && !isblock(p21)) {
    Pos nd;
    nd.x_ = d.x_ - d.y_;
    nd.y_ = d.y_ - d.x_;
    cb(p21, nd);
  }
}
bool Jps::addfn(State s, Pos d) {
  bool b = false;
  Pos ps{s.x_, s.y_};
  fneig(ps, d, [&](Pos np, Pos nd) {
    State ns{np.x_, np.y_, s.cost_ + HWEI, 0, nd.x_, nd.y_};
    b = addjp(ns, ps);
  });
  return b;
}

bool Jps::addjp(State s, Pos pre) {
  Pos ps{s.x_, s.y_};
  if (pre_.find(ps) != pre_.end()) return false;
  if (ps == end_) {
    s.weigh_ = 0;
  } else {
    if (quick_) {
      s.weigh_ = cost(ps, end_);
    } else {
      s.weigh_ = cost(ps, end_) + s.cost_;
    }
  }
  cout << "addjp:" << s.x_ << "," << s.y_ << " ";
  cout << s.dx_ << "," << s.dy_ << " " << s.cost_ << " ";
  cout << pre.x_ << "," << pre.y_ << endl;
  openlist_.push(s);
  pre_[ps] = pre;
  return true;
}

void Jps::genret() {
  Pos p = end_;
  ret_.push_back(end_);
  while (true) {
    auto it = pre_.find(p);
    if (it == pre_.end()) return;
    p = it->second;
    int rsize = ret_.size();
    if (rsize >= 2) {
      Pos pp = ret_[rsize - 1];
      Pos ppp = ret_[rsize - 2];
      int16_t dx1 = pp.x_ - p.x_;
      int16_t dy1 = pp.y_ - p.y_;
      int16_t dx2 = ppp.x_ - p.x_;
      int16_t dy2 = ppp.y_ - p.y_;
      if (dx1 * dy2 == dy1 * dx2) {
        ret_[rsize - 1] = p;
      } else {
        ret_.push_back(p);
      }
    } else {
      ret_.push_back(p);
    }
  }
}

bool Jps::checkend(Pos p1, Pos p2) {
  if (p1 == end_ || p2 == end_) return true;
  Pos v1e;
  v1e.x_ = end_.x_ - p1.x_;
  v1e.y_ = end_.y_ - p1.y_;
  Pos ve2;
  ve2.x_ = p2.x_ - end_.x_;
  ve2.y_ = p2.y_ - end_.y_;
  if (v1e.x_ * ve2.x_ + v1e.y_ * ve2.y_ < 0) return false;
  if (v1e.x_ * ve2.y_ == v1e.y_ * ve2.x_) return true;
  return false;
}

bool Jps::step(State st, Pos d) {
  st.dx_ = d.x_;
  st.dy_ = d.y_;
  Pos pstart{st.x_, st.y_};
  State s = st;
  cout << "step: " << st.x_ << "," << st.y_;
  cout << " " << d.x_ << " " << d.y_ << endl;

  while (true) {
    if (d.x_ == 0 || d.y_ == 0) {
      Pos ps{.x_ = s.x_, .y_ = s.y_};
      if (isblock(ps)) return false;
      int16_t len = getcache(ps, d);
      if (len <= 0) return false;

      State ns = s;
      ns.x_ += d.x_ * len;
      ns.y_ += d.y_ * len;
      ns.cost_ += LWEI * len;
      Pos pe{.x_ = ns.x_, .y_ = ns.y_};
      if (checkend(ps, pe)) {
        addjp({end_.x_, end_.y_}, ps);
        return true;
      }
      if (addfn(ns, d)) {
        addjp(ns, ps);
        return true;
      }
      return false;
    } else if (d.x_ != 0 && d.y_ != 0) {
      Pos ps = {s.x_, s.y_};
      bool b1 = addfn(s, {d.x_, 0});
      bool b2 = addfn(s, {0, d.y_});
      if (b1 || b2) {
        if (!(ps == pstart) && pre_.find(ps) == pre_.end()) pre_[ps] = pstart;
        s.x_ += d.x_;
        s.y_ += d.y_;
        s.cost_ += HWEI;
        addjp(s, pstart);
        return true;
      }
      s.x_ += d.x_;
      s.y_ += d.y_;
      s.cost_ += HWEI;
      cout << "di: " << s.x_ << "," << s.y_ << endl;
      ps = {s.x_, s.y_};
      if (ps == end_) {
        addjp(s, pstart);
        return true;
      }
      if (isblock(ps)) return false;
      if (pre_.find(ps) != pre_.end()) return false;
      bool r1 = step(s, {d.x_, 0});
      bool r2 = step(s, {0, d.y_});
      if (r1 || r2) {
        if (pre_.find(ps) == pre_.end()) pre_[ps] = pstart;
        s.x_ += d.x_;
        s.y_ += d.y_;
        s.cost_ += HWEI;
        addjp(s, pstart);
        return true;
      }
    }
  }
}

void Jps::find() {
  if (isblock(start_) || isblock(end_)) return;
  if (start_ == end_) return;
  State sstate{start_.x_, start_.y_, 0, cost(start_, end_), 0, 0};
  openlist_.push(sstate);

  while (!openlist_.empty()) {
    State st = openlist_.top();
    openlist_.pop();
    Pos pt{.x_ = st.x_, .y_ = st.y_};
    if (closelist_.find(pt) != closelist_.end()) continue;
    closelist_.insert(pt);

    if (end_ == pt) {
      genret();
      return;
    }

    int8_t dx = st.dx_;
    int8_t dy = st.dy_;
    vector<Pos> dirs;
    dirs.reserve(8);
    if (dx == 0 && dy == 0) {
      dirs = DIR;
    } else if (dx != 0 && dy != 0) {
      dirs.push_back({dx, 0});
      dirs.push_back({0, dy});
      dirs.push_back({dx, dy});
    } else {
      dirs.push_back({dx, dy});
    }

    for (Pos dir : dirs) {
      step(st, dir);
    }
  }
}