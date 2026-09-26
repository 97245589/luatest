extern "C" {
#include "lauxlib.h"
}
#include "jps.h"

static const char* META = "JPS";
struct Ljps {
  static int create(lua_State*);
  static int gc(lua_State*);

  static int setblock(lua_State*);
  static int jpcache(lua_State*);
  static int dumpcache(lua_State*);
  static int find(lua_State*);
};

int Ljps::dumpcache(lua_State* L) {
  Jps** pp = (Jps**)luaL_checkudata(L, 1, META);
  int16_t dx = luaL_checkinteger(L, 2);
  int16_t dy = luaL_checkinteger(L, 3);

  Jps& jps = **pp;
  string ret = jps.dumpcache({dx, dy});
  lua_pushlstring(L, ret.data(), ret.size());
  return 1;
}

int Ljps::find(lua_State* L) {
  Jps** pp = (Jps**)luaL_checkudata(L, 1, META);
  int16_t sx = luaL_checkinteger(L, 2);
  int16_t sy = luaL_checkinteger(L, 3);
  int16_t ex = luaL_checkinteger(L, 4);
  int16_t ey = luaL_checkinteger(L, 5);
  bool quick = lua_toboolean(L, 6);

  Jps& jps = **pp;
  jps.start_ = {sx, sy};
  jps.end_ = {ex, ey};
  jps.quick_ = quick;
  jps.find();
  vector<Jps::Pos>& ret = jps.ret_;
  int rsize = ret.size();
  if (rsize <= 0) {
    jps.reset();
    return 0;
  }
  int c = 0;
  lua_createtable(L, rsize * 2, 0);
  for (int i = rsize - 1; i >= 0; --i) {
    Jps::Pos p = ret[i];
    lua_pushinteger(L, p.x_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, p.y_);
    lua_rawseti(L, -2, ++c);
  }
  jps.reset();
  return 1;
}

int Ljps::jpcache(lua_State* L) {
  Jps** pp = (Jps**)luaL_checkudata(L, 1, META);
  Jps& jps = **pp;
  jps.jpcache();
  return 0;
}

int Ljps::setblock(lua_State* L) {
  Jps** pp = (Jps**)luaL_checkudata(L, 1, META);
  int16_t x = luaL_checkinteger(L, 2);
  int16_t y = luaL_checkinteger(L, 3);
  bool v = lua_toboolean(L, 4);
  Jps& jps = **pp;
  jps.setblock(x, y, v);
  return 0;
}

int Ljps::gc(lua_State* L) {
  Jps** pp = (Jps**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

int Ljps::create(lua_State* L) {
  int16_t len = luaL_checkinteger(L, 1);
  int16_t wid = luaL_checkinteger(L, 2);
  Jps* p = new Jps(len, wid);
  Jps** pp = (Jps**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"setblock", setblock},
                    {"jpcache", jpcache},
                    {"dumpcache", dumpcache},
                    {"find", find},
                    {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_ljps(lua_State* L) {
  luaL_Reg l[] = {{"create", Ljps::create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}