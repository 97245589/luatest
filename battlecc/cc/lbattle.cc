extern "C" {
#include "lauxlib.h"
}
#include <iostream>

#include "battle.h"
using namespace std;

static const char* META = "LBATTLE";

static int setinfo(lua_State* L) {
  Battle** pp = (Battle**)luaL_checkudata(L, 1, META);
  Battle& battle = **pp;
  auto& atk = battle.atk_;
  auto& def = battle.def_;
  atk.clear();
  def.clear();
  atk.reserve(3);
  def.reserve(3);
  Actor actor;
  auto& attrs = actor.attrs_;
  attrs[HP] = 100;
  attrs[ATK] = 20;
  attrs[DEF] = 10;
  actor.lo_ = 2;
  atk.push_back(actor);
  def.push_back(actor);
  actor.lo_ = 5;
  atk.push_back(actor);
  def.push_back(actor);
  return 0;
}

static int start(lua_State* L) {
  Battle** pp = (Battle**)luaL_checkudata(L, 1, META);
  Battle& battle = **pp;
  battle.start();
  return 0;
}

static int skillcfg(lua_State* L) {
  Battle** pp = (Battle**)luaL_checkudata(L, 1, META);
  Battle& battle = **pp;
  auto& skillcfg = battle.skill_.skillcfg_;
  skillcfg[100] = {.targ_ = "enemy", .aparams_ = {2}};
  skillcfg[200] = {.targ_ = "me", .aparams_ = {2}};
  skillcfg[300] = {.targ_ = "me", .aparams_ = {ATK, 10}};
  skillcfg[400] = {.targ_ = "me", .aparams_ = {10}};
  return 0;
}

static int gc(lua_State* L) {
  Battle** pp = (Battle**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

static int create(lua_State* L) {
  Battle* p = new Battle();
  Battle** pp = (Battle**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"setinfo", setinfo},
                    {"start", start},
                    {"skillcfg", skillcfg},
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
LUAMOD_API int luaopen_lbattle(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}