extern "C" {
#include "lauxlib.h"
}
#include <iostream>
#include <tuple>
using namespace std;

#include "map.h"

static const char* META = "MAP";

static tuple<Pos, Pos> bl_tr(lua_State* L, int idx) {
  int16_t blx = luaL_checkinteger(L, idx);
  int16_t bly = luaL_checkinteger(L, idx + 1);
  int16_t trx = luaL_checkinteger(L, idx + 2);
  int16_t try_ = luaL_checkinteger(L, idx + 3);
  Pos bl{.x_ = blx, .y_ = bly};
  Pos tr{.x_ = trx, .y_ = try_};
  return make_tuple(bl, tr);
}

static int add_obstacle(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  auto [bl, tr] = bl_tr(L, 2);
  Map& m = **pp;
  m.add_obstalce(bl, tr);
  return 0;
}

static int del_obstacle(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  auto [bl, tr] = bl_tr(L, 2);
  Map& m = **pp;
  m.del_obstacle(bl, tr);
  return 0;
}

static int add_entity(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t eid = luaL_checkinteger(L, 2);
  auto [bl, tr] = bl_tr(L, 3);
  Map& m = **pp;
  m.add_entity({.id_ = eid, .bl_ = bl, .tr_ = tr});
  return 0;
}

static int del_entity(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t eid = luaL_checkinteger(L, 2);
  Map& m = **pp;
  m.del_entity(eid);
  return 0;
}

static int area_obstacle(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  auto [bl, tr] = bl_tr(L, 2);
  Map& m = **pp;
  bool b = m.area_obstacle(bl, tr);
  lua_pushboolean(L, b);
  return 1;
}

static int area_eid(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  auto [bl, tr] = bl_tr(L, 2);
  Map& m = **pp;
  vector<int64_t> ret;
  m.area_eid(bl, tr, ret);
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (int64_t id : ret) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int set_vision(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  auto [bl, tr] = bl_tr(L, 3);
  Map& m = **pp;
  Vision v{.id_ = id, .bl_ = bl, .tr_ = tr};
  m.set_vision(v);
  return 0;
}

static int del_vision(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  Map& m = **pp;
  m.del_vision(id);
  return 0;
}

static int area_vid(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  auto [bl, tr] = bl_tr(L, 2);
  Map& m = **pp;
  vector<int64_t> ids;
  m.area_vid(bl, tr, ids);
  if (ids.empty()) return 0;
  lua_createtable(L, ids.size(), 0);
  int c = 0;
  for (int64_t id : ids) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int add_march(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t vid = luaL_checkinteger(L, 2);
  auto [cur, end] = bl_tr(L, 3);
  Map& m = **pp;
  m.add_march({.id_ = vid, .cur_ = cur, .end_ = end});
  return 0;
}

static int clear_march(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  Map& m = **pp;
  m.clear_march();
  return 0;
}

static int vision_march(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t vid = luaL_checkinteger(L, 2);
  Map& m = **pp;
  vector<int64_t> ret;
  m.vision_march(vid, ret);
  if (ret.empty()) return 0;
  int c = 0;
  lua_createtable(L, ret.size(), 0);
  for (int64_t mid : ret) {
    lua_pushinteger(L, mid);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int gc(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

static int create(lua_State* L) {
  int16_t len = luaL_checkinteger(L, 1);
  int16_t wid = luaL_checkinteger(L, 2);
  Map* p = new Map(len, wid);
  Map** pp = (Map**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"add_obstacle", add_obstacle},
                    {"del_obstacle", del_obstacle},
                    {"add_entity", add_entity},
                    {"del_entity", del_entity},
                    {"area_obstacle", area_obstacle},
                    {"area_eid", area_eid},
                    {"add_march", add_march},
                    {"clear_march", clear_march},
                    {"set_vision", set_vision},
                    {"del_vision", del_vision},
                    {"area_vid", area_vid},
                    {"vision_march", vision_march},
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
LUAMOD_API int luaopen_lmap(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}