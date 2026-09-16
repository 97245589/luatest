extern "C" {
#include "lauxlib.h"
}
#include <iostream>
#include <unordered_map>

#include "dbimpl.h"
using namespace std;

static int compact(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  p->compact();
  return 0;
}

static int keys(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lp;
  const char* pp = luaL_checklstring(L, 2, &lp);
  string patt(pp, lp);
  vector<string> ret;
  p->keys(patt, ret);
  lua_createtable(L, ret.size(), 0);
  size_t c = 0;
  for (const string& key : ret) {
    lua_pushlstring(L, key.data(), key.size());
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int del(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);
  p->del(key);
  return 0;
}

static int hmset(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);

  vector<string> fv;
  fv.reserve(16);
  int pnum = lua_gettop(L);
  if (0 != pnum % 2) return luaL_error(L, "hmset err");
  for (int i = 3; i < pnum; i += 2) {
    size_t lf, lv;
    const char* pf = luaL_checklstring(L, i, &lf);
    const char* pv = luaL_checklstring(L, i + 1, &lv);
    fv.push_back({pf, lf});
    fv.push_back({pv, lv});
  }
  p->hmset(key, fv);
  return 0;
}

static int hdel(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);
  vector<string> fields;
  fields.reserve(16);
  int pnum = lua_gettop(L);
  for (int i = 3; i <= pnum; ++i) {
    size_t lf;
    const char* pf = luaL_checklstring(L, i, &lf);
    fields.push_back({pf, lf});
  }
  p->hdel(key, fields);
  return 0;
}

static int hgetall(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);
  vector<string> ret;
  p->hgetall(key, ret);
  lua_createtable(L, ret.size(), 0);
  int c = 0;
  for (const string& e : ret) {
    lua_pushlstring(L, e.data(), e.size());
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

static int hget(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk, lf;
  const char* pk = luaL_checklstring(L, 2, &lk);
  const char* pf = luaL_checklstring(L, 3, &lf);
  string key(pk, lk);
  vector<string> fields;
  fields.push_back({pf, lf});
  vector<bool> valid;
  p->hmget(key, fields, valid);
  if (!valid[0]) return 0;
  const string& val = fields[0];
  lua_pushlstring(L, val.data(), val.size());
  return 1;
}

static int hmget(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);
  int pnum = lua_gettop(L);
  vector<string> fields;
  fields.reserve(16);
  for (int i = 3; i <= pnum; ++i) {
    size_t lf;
    const char* pf = luaL_checklstring(L, i, &lf);
    fields.push_back({pf, lf});
  }
  vector<bool> valid;
  p->hmget(key, fields, valid);
  lua_createtable(L, fields.size(), 0);
  for (int i = 0; i < fields.size(); ++i) {
    if (valid[i]) {
      const string& field = fields[i];
      lua_pushlstring(L, field.data(), field.size());
      lua_rawseti(L, -2, i + 1);
    }
  }
  return 1;
}

static int hscan(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string key(pk, lk);
  string cursor;
  if (!lua_isinteger(L, 3)) {
    size_t lc;
    const char* pc = luaL_checklstring(L, 3, &lc);
    cursor = {pc, lc};
  }
  int pnum = lua_gettop(L);
  if (1 != pnum % 2) return luaL_error(L, "hscan err");
  string match = "*";
  int count = 10;
  for (int i = 4; i < pnum; i += 2) {
    size_t ls;
    const char* ps = luaL_checklstring(L, i, &ls);
    string str(ps, ls);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (str == "match") {
      size_t lm;
      const char* pm = luaL_checklstring(L, i + 1, &lm);
      match = {pm, lm};
    } else if (str == "count") {
      count = luaL_checkinteger(L, i + 1);
    }
  }

  vector<string> ret;
  p->hscan(key, cursor, match, count, ret);
  lua_createtable(L, 2, 0);
  const string& rcursor = ret[0];
  lua_pushlstring(L, rcursor.data(), rcursor.size());
  lua_rawseti(L, -2, 1);
  lua_createtable(L, ret.size(), 0);
  for (int i = 1; i < ret.size(); ++i) {
    const string& val = ret[i];
    lua_pushlstring(L, val.data(), val.size());
    lua_rawseti(L, -2, i);
  }
  lua_rawseti(L, -2, 2);
  return 1;
}

static int create(lua_State* L) {
  size_t len;
  const char* ps = luaL_checklstring(L, 1, &len);
  string name(ps, len);
  int64_t cache_size = lua_tointeger(L, 2);

  Dbimpl* p = new Dbimpl();
  bool ok = p->open(name, cache_size);
  if (!ok) {
    delete p;
    return luaL_error(L, "db create err");
  }
  lua_pushlightuserdata(L, p);
  return 1;
}

static int release(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Dbimpl* p = (Dbimpl*)lua_touserdata(L, 1);
  delete p;
  return 0;
}

extern "C" {
LUAMOD_API int luaopen_ldb(lua_State* L) {
  luaL_Reg l[] = {
      {"create", create}, {"release", release}, {"compact", compact},
      {"keys", keys},     {"del", del},         {"hmset", hmset},
      {"hset", hmset},    {"hdel", hdel},       {"hgetall", hgetall},
      {"hget", hget},     {"hmget", hmget},     {"hscan", hscan},
      {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}