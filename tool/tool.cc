extern "C" {
#include "lauxlib.h"
}
#include <string>
using namespace std;

static constexpr int8_t MAXDEP = 10;
struct Dump {
  lua_State* L;
  int dep;
  int pdep;
  string buf;

  void write(const string& cont) { buf.append(cont); }
  void dump(int idx);
  void dumptable(int idx);
};
void Dump::dumptable(int idx) {
  ++dep;
  if (dep > MAXDEP || (pdep > 0 && dep > pdep)) {
    write("MAXDEP");
    --dep;
    return;
  }
  if (idx < 0) idx = lua_gettop(L) + idx + 1;
  lua_checkstack(L, 2);
  write("{\n");
  lua_pushnil(L);
  while (lua_next(L, idx) != 0) {
    write(string(dep * 2, ' '));
    write("[");
    dump(-2);
    write("] = ");
    dump(-1);
    write(",\n");
    lua_pop(L, 1);
  }
  write(string(dep * 2 - 2, ' '));
  write("}");
  --dep;
}
void Dump::dump(int idx) {
  int type = lua_type(L, idx);
  switch (type) {
    case LUA_TBOOLEAN: {
      bool v = lua_toboolean(L, idx);
      if (v) {
        write("true");
      } else {
        write("false");
      }
      return;
    }
    case LUA_TNUMBER: {
      if (lua_isinteger(L, idx)) {
        int64_t v = lua_tointeger(L, idx);
        write(to_string(v));
      } else {
        double v = lua_tonumber(L, idx);
        write(to_string(v));
      }
      return;
    }
    case LUA_TSTRING: {
      size_t len;
      const char* p = lua_tolstring(L, idx, &len);
      string v;
      v.reserve(len + 2);
      v.push_back('"');
      v.append(p, len);
      v.push_back('"');
      write(v);
      return;
    }
    case LUA_TTABLE: {
      dumptable(idx);
      return;
    }
    default: {
      write("--");
      return;
    }
  }
}
static int dump(lua_State* L) {
  int pdep = 0;
  if (lua_isinteger(L, 2)) pdep = lua_tointeger(L, 2);
  lua_settop(L, 1);
  Dump d{.L = L, .dep = 0, .pdep = pdep};
  string& buf = d.buf;
  buf.reserve(1024);
  d.dump(1);
  lua_pushlstring(L, buf.data(), buf.size());
  return 1;
}

struct Clone {
  lua_State* L;
  int dep;

  void clone(int idx);
  void clonetable(int idx);
};
void Clone::clonetable(int idx) {
  ++dep;
  if (dep > MAXDEP) {
    luaL_error(L, "clone dep err");
    return;
  }
  if (idx < 0) idx = lua_gettop(L) + idx + 1;
  lua_createtable(L, 0, 0);
  int nidx = lua_gettop(L);
  lua_checkstack(L, 4);
  lua_pushnil(L);
  while (lua_next(L, idx) != 0) {
    clone(-2);
    clone(-2);
    lua_settable(L, nidx);
    lua_pop(L, 1);
  }
  --dep;
}
void Clone::clone(int idx) {
  int type = lua_type(L, idx);
  switch (type) {
    case LUA_TBOOLEAN:
    case LUA_TNUMBER:
    case LUA_TSTRING: {
      lua_pushvalue(L, idx);
      return;
    }
    case LUA_TTABLE: {
      clonetable(idx);
      return;
    }
    default: {
      luaL_error(L, "clonetype err %d", type);
      return;
    }
  }
}
static int clone(lua_State* L) {
  lua_settop(L, 1);
  Clone c{.L = L, .dep = 0};
  c.clone(1);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_ltool(lua_State* L) {
  luaL_Reg l[] = {{"dump", dump}, {"clone", clone}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}