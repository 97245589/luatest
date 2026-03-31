extern "C" {
#include "lauxlib.h"
}
#include <iostream>
#include <sstream>

#include "world.h"
using namespace std;

static const char* META = "LWORLD";
struct Lworld {
  static int create(lua_State*);
  static int gc(lua_State*);

  static int useskill(lua_State*);
  static int addactor(lua_State*);
  static int delactor(lua_State*);
  static int dumpaoi(lua_State*);
  static int setpos(lua_State*);
  static int aoiids(lua_State*);
  static int setblock(lua_State*);
  static int astar(lua_State*);
};

int Lworld::useskill(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  int skillid = luaL_checkinteger(L, 3);
  World& world = **pp;
  world.useskill(id, skillid);
  return 0;
}

int Lworld::dumpaoi(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  World& world = **pp;
  auto& aoi = world.aoi_.aoi_;
  ostringstream oss;
  oss << "aoiinfo:" << endl;
  for (auto& [p, ids] : aoi) {
    oss << p.x_ << "," << p.y_ << "->";
    for (int64_t id : ids) {
      oss << id << " ";
    }
    oss << endl;
  }
  const string& str = oss.str();
  lua_pushlstring(L, str.data(), str.size());
  return 1;
}

int Lworld::aoiids(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);

  World& world = **pp;
  vector<int64_t> ret;
  world.aoi_.aoi_ids(id, ret);
  if (ret.empty()) return 0;
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (int64_t id : ret) {
    lua_pushinteger(L, id);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

int Lworld::setblock(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int16_t x = luaL_checkinteger(L, 2);
  int16_t y = luaL_checkinteger(L, 3);
  int8_t v = luaL_checkinteger(L, 4);

  World& world = **pp;
  world.setblock({x, y}, v);
  return 0;
}

int Lworld::astar(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int16_t sx = luaL_checkinteger(L, 2);
  int16_t sy = luaL_checkinteger(L, 3);
  int16_t ex = luaL_checkinteger(L, 4);
  int16_t ey = luaL_checkinteger(L, 5);
  bool quick = lua_toboolean(L, 6);
  World& world = **pp;
  vector<Pos> ret;
  world.astar_.find({sx, sy}, {ex, ey}, ret, quick);
  if (ret.empty()) return 0;
  int rsize = ret.size();
  lua_createtable(L, rsize * 2, 0);
  int c = 0;
  for (int i = rsize - 1; i >= 0; --i) {
    auto p = ret[i];
    lua_pushinteger(L, p.x_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, p.y_);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

int Lworld::addactor(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  float x = luaL_checknumber(L, 3);
  float y = luaL_checknumber(L, 4);
  int16_t dx = luaL_checkinteger(L, 5);
  int16_t dy = luaL_checkinteger(L, 6);
  int8_t camp = luaL_checkinteger(L, 7);

  World& world = **pp;
  Actor actor{id, x, y, dx, dy, camp};
  world.addactor(id, actor);
  return 0;
}

int Lworld::setpos(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  float x = luaL_checknumber(L, 3);
  float y = luaL_checknumber(L, 4);
  int16_t dx = luaL_checkinteger(L, 5);
  int16_t dy = luaL_checkinteger(L, 6);
  World& world = **pp;
  vector<int64_t> adds;
  vector<int64_t> dels;
  world.setpos(id, x, y, dx, dy, adds, dels);
  if (adds.empty()) {
    lua_pushnil(L);
  } else {
    int asize = adds.size();
    lua_createtable(L, asize, 0);
    int c = 0;
    for (int64_t id : adds) {
      lua_pushinteger(L, id);
      lua_rawseti(L, -2, ++c);
    }
  }
  if (dels.empty()) {
    lua_pushnil(L);
  } else {
    int dsize = dels.size();
    lua_createtable(L, dsize, 0);
    int c = 0;
    for (int64_t id : dels) {
      lua_pushinteger(L, id);
      lua_rawseti(L, -2, ++c);
    }
  }
  return 2;
}

int Lworld::delactor(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  int64_t id = luaL_checkinteger(L, 2);
  World& world = **pp;
  world.delactor(id);
  return 0;
}

int Lworld::gc(lua_State* L) {
  World** pp = (World**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

int Lworld::create(lua_State* L) {
  int16_t x = luaL_checkinteger(L, 1);
  int16_t y = luaL_checkinteger(L, 2);
  World* p = new World(x, y);
  World** pp = (World**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {
        {"useskill", useskill}, {"addactor", addactor}, {"delactor", delactor},
        {"dumpaoi", dumpaoi},   {"setpos", setpos},     {"aoiids", aoiids},
        {"setblock", setblock}, {"astar", astar},       {NULL, NULL}};
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
  luaL_Reg l[] = {{"create", Lworld::create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}