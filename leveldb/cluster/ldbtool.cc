extern "C" {
#include "lauxlib.h"
}
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
using namespace std;
#include "parallel_hashmap/phmap.h"

static const char* VERSION = "VERSION";
struct Version {
  map<int64_t, string> version_rawkey_;
  phmap::flat_hash_map<string, int64_t> rawkey_version_;

  void add(const string& key, int64_t version) {
    if (auto vit = version_rawkey_.find(version);
        vit != version_rawkey_.end()) {
      return;
    }
    if (auto rit = rawkey_version_.find(key); rit != rawkey_version_.end()) {
      version_rawkey_.erase(rit->second);
      rawkey_version_.erase(rit);
    }
    version_rawkey_[version] = key;
    rawkey_version_[key] = version;
  }

  string seri() {
    string ret;
    ret.reserve(1024);
    for (auto& [version, key] : version_rawkey_) {
      ret.append((char*)&version, sizeof(version));
      uint32_t len = key.size();
      ret.append((char*)&len, sizeof(len));
      ret.append(key.data(), key.size());
    }
    return ret;
  }

  void deseri(string_view str) {
    char* pstart = (char*)str.data();
    char* pend = pstart + str.size();
    while (pstart < pend) {
      int64_t version = *(int64_t*)pstart;
      pstart += sizeof(version);
      uint32_t len = *(int32_t*)pstart;
      pstart += sizeof(len);
      char* p = pstart;
      pstart += len;
      add({p, len}, version);
    }
  }

  string dump() {
    ostringstream oss;
    oss << "size :" << version_rawkey_.size() << " " << rawkey_version_.size()
        << endl;
    for (auto& [v, k] : version_rawkey_) {
      oss << v << "," << k << " ";
    }
    oss << endl;
    return oss.str();
  }

  static int add(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    size_t ls;
    const char* ps = luaL_checklstring(L, 2, &ls);
    string key(ps, ls);
    int64_t version = luaL_checkinteger(L, 3);
    ver.add(key, version);
    return 0;
  }

  static int last(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    auto& version_rawkey = ver.version_rawkey_;
    if (version_rawkey.empty()) return 0;
    auto it = prev(version_rawkey.end());
    lua_pushinteger(L, it->first);
    string& str = it->second;
    lua_pushlstring(L, str.data(), str.size());
    return 2;
  }

  static int version_after(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    int64_t version = luaL_checkinteger(L, 2);
    int num = 100;
    if (lua_isinteger(L, 3)) num = lua_tointeger(L, 3);

    Version& ver = **pp;
    auto& version_rawkey = ver.version_rawkey_;
    lua_createtable(L, num, 0);
    int64_t rversion;
    int c = 0;
    for (auto it = version_rawkey.upper_bound(version);
         it != version_rawkey.end(); ++it) {
      string& rawkey = it->second;
      lua_pushlstring(L, rawkey.data(), rawkey.size());
      lua_rawseti(L, -2, ++c);
      rversion = it->first;
      if (c >= num) break;
    }
    if (0 == c) return 0;
    lua_pushinteger(L, rversion);
    lua_rawseti(L, -2, ++c);
    return 1;
  }

  static int dump(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    string ret = ver.dump();
    lua_pushlstring(L, ret.data(), ret.size());
    return 1;
  }

  static int size(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    lua_pushinteger(L, ver.rawkey_version_.size());
    return 1;
  }

  static int seri(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    string ret = ver.seri();
    lua_pushlstring(L, ret.data(), ret.size());
    return 1;
  }

  static int deseri(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    Version& ver = **pp;
    size_t ls;
    const char* ps = luaL_checklstring(L, 2, &ls);
    ver.deseri({ps, ls});
    return 0;
  }

  static int gc(lua_State* L) {
    Version** pp = (Version**)luaL_checkudata(L, 1, VERSION);
    delete *pp;
    return 0;
  }

  static int create(lua_State* L) {
    Version* p = new Version();
    Version** pp = (Version**)lua_newuserdata(L, sizeof(p));
    *pp = p;
    if (luaL_newmetatable(L, VERSION)) {
      luaL_Reg l[] = {{"add", add},       {"version_after", version_after},
                      {"last", last},     {"size", size},
                      {"dump", dump},     {"seri", seri},
                      {"deseri", deseri}, {NULL, NULL}};
      luaL_newlib(L, l);
      lua_setfield(L, -2, "__index");
      lua_pushcfunction(L, gc);
      lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
    return 1;
  }
};

static const char* DUMPF = "DUMPF";
struct Dumpf {
  set<int64_t> set_;

  static int add(lua_State* L) {
    Dumpf** pp = (Dumpf**)luaL_checkudata(L, 1, DUMPF);
    int64_t v = luaL_checkinteger(L, 2);
    Dumpf& f = **pp;
    f.set_.insert(v);
    return 0;
  }
  static int del(lua_State* L) {
    Dumpf** pp = (Dumpf**)luaL_checkudata(L, 1, DUMPF);
    int64_t v = luaL_checkinteger(L, 2);
    Dumpf& f = **pp;
    f.set_.erase(v);
    return 0;
  }
  static int find_less(lua_State* L) {
    Dumpf** pp = (Dumpf**)luaL_checkudata(L, 1, DUMPF);
    int64_t v = luaL_checkinteger(L, 2);
    Dumpf& f = **pp;
    auto& s = f.set_;
    auto it = s.upper_bound(v);
    if (it == s.begin()) return 0;
    --it;
    lua_pushinteger(L, *it);
    return 1;
  }
  static int dump(lua_State* L) {
    Dumpf** pp = (Dumpf**)luaL_checkudata(L, 1, DUMPF);
    Dumpf& f = **pp;
    auto& s = f.set_;
    ostringstream oss;
    oss << "size: " << s.size() << endl;
    for (int64_t v : s) {
      oss << v << " ";
    }
    oss << endl;
    string ret = oss.str();
    lua_pushlstring(L, ret.data(), ret.size());
    return 1;
  }
  static int gc(lua_State* L) {
    Dumpf** pp = (Dumpf**)luaL_checkudata(L, 1, DUMPF);
    delete *pp;
    return 0;
  }
  static int create(lua_State* L) {
    Dumpf* p = new Dumpf();
    Dumpf** pp = (Dumpf**)lua_newuserdata(L, sizeof(p));
    *pp = p;
    if (luaL_newmetatable(L, DUMPF)) {
      luaL_Reg l[] = {{"add", add},
                      {"del", del},
                      {"find_less", find_less},
                      {"dump", dump},
                      {NULL, NULL}};
      luaL_newlib(L, l);
      lua_setfield(L, -2, "__index");
      lua_pushcfunction(L, gc);
      lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
    return 1;
  }
};

static const char* SLOT = "SLOT";
struct Slot {
  map<int, int> slot_group_;

  static int find_group(lua_State* L) {
    Slot** pp = (Slot**)luaL_checkudata(L, 1, SLOT);
    int val = luaL_checkinteger(L, 2);
    Slot& slot = **pp;
    auto& slot_group = slot.slot_group_;
    auto it = slot_group.lower_bound(val);
    if (it == slot_group.end()) {
      auto bit = slot_group.begin();
      if (bit == slot_group.end()) return 0;
      lua_pushinteger(L, bit->second);
      return 1;
    } else {
      lua_pushinteger(L, it->second);
      return 1;
    }
    return 0;
  }

  static int gc(lua_State* L) {
    Slot** pp = (Slot**)luaL_checkudata(L, 1, SLOT);
    delete *pp;
    return 0;
  }

  static int create(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    map<int, int> cfg;
    lua_pushnil(L);
    while (0 != lua_next(L, 1)) {
      int slot = luaL_checkinteger(L, -2);
      int group = luaL_checkinteger(L, -1);
      cfg[slot] = group;
      lua_pop(L, 1);
    }
    Slot* p = new Slot();
    p->slot_group_ = std::move(cfg);
    Slot** pp = (Slot**)lua_newuserdata(L, sizeof(p));
    *pp = p;
    if (luaL_newmetatable(L, SLOT)) {
      luaL_Reg l[] = {{"find_group", find_group}, {NULL, NULL}};
      luaL_newlib(L, l);
      lua_setfield(L, -2, "__index");
      lua_pushcfunction(L, gc);
      lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
    return 1;
  }
};

extern "C" {
LUAMOD_API int luaopen_ldbtool(lua_State* L) {
  luaL_Reg l[] = {{"create_slot", Slot::create},
                  {"create_version", Version::create},
                  {"create_dumpf", Dumpf::create},
                  {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}