extern "C" {
#include "lauxlib.h"
}
#include "world.h"

static const char* META = "LWORLD";

static int setentity(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  World& world = **pp;
  int64_t id = luaL_checkinteger(L, 2);
  if (!lua_isinteger(L, 3)) {
    world.delentity(id);
    return 0;
  }
  int16_t blx = luaL_checkinteger(L, 3);
  int16_t bly = luaL_checkinteger(L, 4);
  int16_t trx = luaL_checkinteger(L, 5);
  int16_t try_ = luaL_checkinteger(L, 6);
  world.addentity(id, {{blx, bly}, {trx, try_}});
  return 0;
}

static int haveblock(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int16_t blx = luaL_checkinteger(L, 2);
  int16_t bly = luaL_checkinteger(L, 3);
  int16_t trx = luaL_checkinteger(L, 4);
  int16_t try_ = luaL_checkinteger(L, 5);

  World& world = **pp;
  bool r = world.haveblock({blx, bly}, {trx, try_});
  lua_pushboolean(L, r);
  return 1;
}

static int setblock(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int16_t x = luaL_checkinteger(L, 2);
  int16_t y = luaL_checkinteger(L, 3);
  bool v = lua_toboolean(L, 4);
  World& world = **pp;
  world.setblock({x, y}, v);
  return 0;
}

static int areaids(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int16_t blx = luaL_checkinteger(L, 2);
  int16_t bly = luaL_checkinteger(L, 3);
  int16_t trx = luaL_checkinteger(L, 4);
  int16_t try_ = luaL_checkinteger(L, 5);

  World& world = **pp;
  vector<int64_t> ret;
  world.areaids({blx, bly}, {trx, try_}, ret);
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (int64_t id : ret) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int settroop(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  World& world = **pp;
  int64_t id = luaL_checkinteger(L, 2);
  if (!lua_isnumber(L, 3)) {
    world.troop_.erase(id);
    return 0;
  }
  int16_t nx = luaL_checkinteger(L, 3);
  int16_t ny = luaL_checkinteger(L, 4);
  int16_t ex = luaL_checkinteger(L, 5);
  int16_t ey = luaL_checkinteger(L, 6);
  world.troop_[id] = {{nx, ny}, {ex, ey}};
  return 0;
}

static int setview(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  World& world = **pp;
  int64_t id = luaL_checkinteger(L, 2);
  if (!lua_isnumber(L, 3)) {
    world.view_.erase(id);
    return 0;
  }
  int16_t blx = luaL_checkinteger(L, 3);
  int16_t bly = luaL_checkinteger(L, 4);
  int16_t trx = luaL_checkinteger(L, 5);
  int16_t try_ = luaL_checkinteger(L, 6);
  auto& view = world.view_[id];
  view.bl_ = {blx, bly};
  view.tr_ = {trx, try_};
  return 0;
}

static int troopview(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  World& world = **pp;

  lua_createtable(L, 0, world.view_.size());
  world.troop_view(
      [&](int64_t id, vector<int64_t>& adds, vector<int64_t> dels) {
        lua_pushinteger(L, id);
        lua_createtable(L, 2, 0);
        if (adds.empty()) {
          lua_pushnil(L);
        } else {
          lua_createtable(L, adds.size(), 0);
          int c = 0;
          for (int64_t aid : adds) {
            lua_pushinteger(L, aid);
            lua_rawseti(L, -2, ++c);
          }
        }
        lua_rawseti(L, -2, 1);
        if (dels.empty()) {
          lua_pushnil(L);
        } else {
          lua_createtable(L, dels.size(), 0);
          int c = 0;
          for (int64_t did : dels) {
            lua_pushinteger(L, did);
            lua_rawseti(L, -2, ++c);
          }
        }
        lua_rawseti(L, -2, 2);
        lua_settable(L, -3);
      });
  return 1;
}

static int gc(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

static int create(lua_State* L) {
  int16_t len = luaL_checkinteger(L, 1);
  int16_t wid = luaL_checkinteger(L, 2);
  World* p = new World(len, wid);
  World** pp = (World**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"setentity", setentity}, {"haveblock", haveblock},
                    {"setblock", setblock},   {"areaids", areaids},
                    {"settroop", settroop},   {"setview", setview},
                    {"troopview", troopview}, {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_lworld(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}