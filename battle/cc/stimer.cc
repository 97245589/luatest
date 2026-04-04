extern "C" {
#include "lauxlib.h"
}
#include <cstdint>
#include <set>
#include <vector>
using namespace std;

static const char* META = "STIMER";
struct Stimer {
  struct Ele {
    int64_t id_;
    int64_t buffid_;
    int64_t endtm_;
    bool operator<(const Ele& rhs) const { return endtm_ < rhs.endtm_; }
  };
  multiset<Ele> timer_;
};

static int add(lua_State* L) {
  Stimer** pp = (Stimer**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  int64_t buffid = luaL_checkinteger(L, 3);
  int64_t endtm = luaL_checkinteger(L, 4);

  Stimer& timer = **pp;
  timer.timer_.insert({id, buffid, endtm});
  return 0;
}

static int expire(lua_State* L) {
  Stimer** pp = (Stimer**)luaL_checkudata(L, 1, META);
  int64_t nowtm = luaL_checkinteger(L, 2);
  Stimer& stimer = **pp;
  auto& timer = stimer.timer_;

  vector<int64_t> ret;
  ret.reserve(8);
  for (auto it = timer.begin(); it != timer.end();) {
    auto& v = *it;
    if (nowtm >= v.endtm_) {
      ret.push_back(v.id_);
      ret.push_back(v.buffid_);
      it = timer.erase(it);
    } else {
      break;
    }
  }
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (int64_t v : ret) {
    lua_pushinteger(L, v);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int clear(lua_State* L) {
  Stimer** pp = (Stimer**)luaL_checkudata(L, 1, META);
  Stimer& timer = **pp;
  timer.timer_.clear();
  return 0;
}

static int gc(lua_State* L) {
  Stimer** pp = (Stimer**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

static int create(lua_State* L) {
  Stimer* p = new Stimer();
  Stimer** pp = (Stimer**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {
        {"add", add}, {"expire", expire}, {"clear", clear}, {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_stimer(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}