extern "C" {
#include "lauxlib.h"
}
#include "map.h"

static const char* META = "LMAP";

static int add_obstacle(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int16_t x = luaL_checkinteger(L, 2);
  int16_t y = luaL_checkinteger(L, 3);
  Map& m = **pp;
  m.add_obstacle({x, y});
  return 0;
}

static int del_obstacle(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int16_t x = luaL_checkinteger(L, 2);
  int16_t y = luaL_checkinteger(L, 3);
  Map& m = **pp;
  m.del_obstacle({x, y});
  return 0;
}

static int astar(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int16_t sx = luaL_checkinteger(L, 2);
  int16_t sy = luaL_checkinteger(L, 3);
  int16_t ex = luaL_checkinteger(L, 4);
  int16_t ey = luaL_checkinteger(L, 5);
  bool quick = lua_toboolean(L, 6);
  Map& m = **pp;
  vector<Pos> ret;
  m.astar_.find({sx, sy}, {ex, ey}, ret, quick);
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size() * 2, 0);
  int c = 0;
  for (Pos p : ret) {
    lua_pushinteger(L, p.x_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, p.y_);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int add_entity(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  float x = luaL_checknumber(L, 3);
  float y = luaL_checknumber(L, 4);
  int16_t dx = luaL_checkinteger(L, 5);
  int16_t dy = luaL_checkinteger(L, 6);
  int8_t atk_type = luaL_checkinteger(L, 7);

  Map& m = **pp;
  Entity entity{id, x, y, dx, dy, atk_type};
  m.add_entity(entity);
  return 0;
}

static int update_pos(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  float x = luaL_checknumber(L, 3);
  float y = luaL_checknumber(L, 4);
  int16_t dx = luaL_checkinteger(L, 5);
  int16_t dy = luaL_checkinteger(L, 6);
  Map& m = **pp;
  m.update_pos(id, x, y, dx, dy);
  return 0;
}

static int del_entity(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);

  Map& m = **pp;
  m.del_entity(id);
  return 0;
}

static int diff_aoi(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int16_t ox = luaL_checknumber(L, 2);
  int16_t oy = luaL_checknumber(L, 3);
  int16_t nx = luaL_checknumber(L, 4);
  int16_t ny = luaL_checknumber(L, 5);

  Map& m = **pp;
  vector<int64_t> add, del;
  Pos ocell, ncell;
  ocell.x_ = ox / Map::cell_len_;
  ocell.y_ = oy / Map::cell_len_;
  ncell.x_ = nx / Map::cell_len_;
  ncell.y_ = ny / Map::cell_len_;
  if (ocell == ncell) return 0;

  m.diff_aoi(ocell, ncell, add, del);
  lua_createtable(L, 32, 0);
  int a = 0;
  for (int64_t id : add) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++a);
  }
  lua_createtable(L, 32, 0);
  int d = 0;
  for (int64_t id : del) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++d);
  }
  return 2;
}

static int get_aoi(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  Map& m = **pp;
  auto& entities = m.entities_;
  auto it = entities.find(id);
  if (it == entities.end()) return 0;
  Entity& entity = it->second;
  Pos cell;
  cell.x_ = entity.x_ / Map::cell_len_;
  cell.y_ = entity.y_ / Map::cell_len_;

  int i = 0;
  lua_createtable(L, 32, 0);
  m.get_aoi(cell, [&](Pos cell, Map::IDSET& ids) {
    for (int64_t oid : ids) {
      lua_pushinteger(L, oid);
      lua_rawseti(L, -2, ++i);
    }
  });
  return 1;
}

static int aoi_search(lua_State* L) {
  Map** pp = (Map**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  int num = luaL_checkinteger(L, 3);
  int8_t atk_type = luaL_checkinteger(L, 4);
  int8_t range_tp = luaL_checkinteger(L, 5);
  float p1 = luaL_checknumber(L, 6);
  float p2 = lua_tonumber(L, 7);

  Search info{id, num, atk_type, range_tp, p1, p2};
  Map& m = **pp;
  vector<int64_t> ret;
  m.aoi_search(info, ret);
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (int64_t id : ret) {
    lua_pushinteger(L, id);
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
    luaL_Reg l[] = {{"add_entity", add_entity},
                    {"update_pos", update_pos},
                    {"del_entity", del_entity},
                    {"aoi_search", aoi_search},
                    {"get_aoi", get_aoi},
                    {"diff_aoi", diff_aoi},
                    {"add_obstacle", add_obstacle},
                    {"del_obstacle", del_obstacle},
                    {"astar", astar},
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