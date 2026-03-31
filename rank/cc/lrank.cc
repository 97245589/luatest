extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <cstdint>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <string>
#include <unordered_map>
using namespace std;

struct Rankele {
  string id_;
  int64_t score_;
  int64_t tm_;

  bool operator<(const Rankele& rhs) const {
    if (score_ != rhs.score_) return score_ > rhs.score_;
    if (tm_ != rhs.tm_) return tm_ < rhs.tm_;
    return id_ < rhs.id_;
  }
};

struct Rank {
  template <class T>
  using ordered_set =
      __gnu_pbds::tree<T, __gnu_pbds::null_type, less<T>,
                       __gnu_pbds::rb_tree_tag,
                       __gnu_pbds::tree_order_statistics_node_update>;

  ordered_set<Rankele> ranks_;
  unordered_map<string, ordered_set<Rankele>::iterator> id_it_;
  int32_t max_;

  void add(const Rankele& ele) {
    if (auto it = id_it_.find(ele.id_); it != id_it_.end()) {
      ranks_.erase(it->second);
      id_it_.erase(ele.id_);
    }
    auto [it, ok] = ranks_.insert(ele);
    id_it_.insert({ele.id_, it});
    evict();
  }

  void evict() {
    if (ranks_.size() <= max_) return;
    auto lastit = prev(ranks_.end());
    id_it_.erase(lastit->id_);
    ranks_.erase(lastit);
  }

  int get_order(const string& id) {
    auto it = id_it_.find(id);
    if (it == id_it_.end()) return -1;
    return ranks_.order_of_key(*it->second) + 1;
  }
};

static const char* META = "LRANK_META";
struct Lrank {
  static int create(lua_State* L);
  static void meta(lua_State* L);
  static int gc(lua_State* L);

  static int add(lua_State* L);
  static int get_order(lua_State* L);
  static int info(lua_State* L);
  static int seri(lua_State* L);
  static int deseri(lua_State* L);
};

int Lrank::seri(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  Rank& rank = **pp;

  string str;
  str.reserve(1024 * 30);
  for (const auto& ele : rank.ranks_) {
    const string& id = ele.id_;
    uint16_t len = id.size();
    str.append((const char*)&len, sizeof(len));
    str.append(id.data(), len);
    int64_t score = ele.score_;
    str.append((const char*)&score, sizeof(score));
    int64_t tm = ele.tm_;
    str.append((const char*)&tm, sizeof(tm));
  }
  lua_pushlstring(L, str.data(), str.size());
  return 1;
}

int Lrank::deseri(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  Rank& rank = **pp;
  size_t len;
  const char* p = luaL_checklstring(L, 2, &len);

  size_t i = 0;
  while (i < len) {
    uint16_t len = *(uint16_t*)(p + i);
    i += sizeof(len);
    string id(p + i, len);
    i += len;
    int64_t score = *(int64_t*)(p + i);
    i += sizeof(score);
    int64_t tm = *(int64_t*)(p + i);
    i += sizeof(tm);
    rank.add({.id_ = id, .score_ = score, .tm_ = tm});
  }
  return 0;
}

int Lrank::info(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  Rank& rank = **pp;
  auto& ranks = rank.ranks_;
  int lb = luaL_checkinteger(L, 2) - 1;
  int ub = luaL_checkinteger(L, 3);
  if (ub > ranks.size()) ub = ranks.size();
  lua_createtable(L, (ub - lb) * 3, 0);
  int c = 0;
  for (auto it = ranks.find_by_order(lb); it != ranks.find_by_order(ub); ++it) {
    const auto& id = it->id_;
    lua_pushlstring(L, id.c_str(), id.size());
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, it->score_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, it->tm_);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

int Lrank::get_order(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  Rank& rank = **pp;
  size_t len;
  const char* p = luaL_checklstring(L, 2, &len);
  int order = rank.get_order({p, len});
  if (order <= 0) return 0;
  lua_pushinteger(L, order);
  return 1;
}

int Lrank::add(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  Rank& rank = **pp;
  size_t len;
  const char* p = luaL_checklstring(L, 2, &len);
  int64_t score = luaL_checkinteger(L, 3);
  int64_t tm = luaL_checkinteger(L, 4);
  rank.add({.id_ = {p, len}, .score_ = score, .tm_ = tm});
  return 0;
}

int Lrank::gc(lua_State* L) {
  Rank** pp = (Rank**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

void Lrank::meta(lua_State* L) {
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"add", add},   {"get_order", get_order}, {"info", info},
                    {"seri", seri}, {"deseri", deseri},       {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Lrank::create(lua_State* L) {
  int num = luaL_checkinteger(L, 1);
  if (num <= 0) return luaL_error(L, "lrank create err");

  Rank* p = new Rank();
  p->max_ = num;
  Rank** pp = (Rank**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  meta(L);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_lrank(lua_State* L) {
  luaL_Reg funcs[] = {{"create", Lrank::create}, {NULL, NULL}};
  luaL_newlib(L, funcs);
  return 1;
}
}