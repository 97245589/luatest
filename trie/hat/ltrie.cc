extern "C" {
#include "lauxlib.h"
}
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
using namespace std;
#include "tsl/htrie_map.h"

static const char* META = "LTRIE";
using Trie = tsl::htrie_map<char, int64_t>;

static int set(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  int64_t v = luaL_checkinteger(L, 3);

  Trie& trie = **pp;
  string_view key(pk, lk);
  trie[key] = v;
  return 0;
}

static int get(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);

  Trie& trie = **pp;
  string_view key(pk, lk);
  auto it = trie.find(key);
  if (it == trie.end()) return 0;
  lua_pushinteger(L, it.value());
  return 1;
}

static int erase(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);

  Trie& trie = **pp;
  string_view key(pk, lk);
  trie.erase(key);
  return 0;
}

static int size(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  Trie& trie = **pp;
  lua_pushinteger(L, trie.size());
  return 1;
}

static int prefix_range(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  size_t lr;
  const char* pr = luaL_checklstring(L, 2, &lr);
  Trie& trie = **pp;
  string_view range(pr, lr);
  auto rit = trie.equal_prefix_range(range);
  string key;
  key.reserve(128);
  lua_createtable(L, 0, 16);
  for (auto it = rit.first; it != rit.second; ++it) {
    it.key(key);
    lua_pushlstring(L, key.data(), key.size());
    lua_pushinteger(L, it.value());
    lua_rawset(L, -3);
  }
  return 1;
}

static int erase_prefix(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  size_t ls;
  const char* ps = luaL_checklstring(L, 2, &ls);
  Trie& trie = **pp;
  string_view pre(ps, ls);
  trie.erase_prefix(pre);
  return 0;
}

static int seri(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  Trie& trie = **pp;
  string ret;
  ret.reserve(1024);
  string key;
  key.reserve(128);
  for (auto it = trie.begin(); it != trie.end(); ++it) {
    it.key(key);
    uint32_t len = key.size();
    ret.append((char*)&len, sizeof(len));
    ret.append(key.data(), key.size());
    int64_t val = it.value();
    ret.append((char*)&val, sizeof(val));
  }
  lua_pushlstring(L, ret.data(), ret.size());
  return 1;
}

static int deseri(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  Trie& trie = **pp;
  size_t ls;
  const char* p = luaL_checklstring(L, 2, &ls);

  char* pstart = (char*)p;
  char* pend = pstart + ls;
  while (pstart < pend) {
    uint32_t slen = *(uint32_t*)pstart;
    pstart += sizeof(slen);
    char* pk = pstart;
    pstart += slen;
    int64_t val = *(int64_t*)pstart;
    pstart += sizeof(val);
    string_view key(pk, slen);
    trie[key] = val;
  }
  return 0;
}

static int gc(lua_State* L) {
  Trie** pp = (Trie**)luaL_checkudata(L, 1, META);
  delete *pp;
  return 0;
}

static int create(lua_State* L) {
  Trie* p = new Trie();
  Trie** pp = (Trie**)lua_newuserdata(L, sizeof(p));
  *pp = p;

  if (luaL_newmetatable(L, META)) {
    luaL_Reg l[] = {{"set", set},
                    {"get", get},
                    {"erase", erase},
                    {"prefix_range", prefix_range},
                    {"erase_prefix", erase_prefix},
                    {"size", size},
                    {"seri", seri},
                    {"deseri", deseri},
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
LUAMOD_API int luaopen_ltrie(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}