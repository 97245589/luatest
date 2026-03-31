extern "C" {
#include "lauxlib.h"
}

// #include <iostream>
#include <list>
#include <string>
#include <unordered_map>
using namespace std;

static const char* LLRU_META = "LLRU_META";
struct Lru {
  list<string> ids_;
  unordered_map<string, list<string>::iterator> it_;
  int num_;
  void update(const string& id, string& evi) {
    del(id);
    ids_.push_front(id);
    it_[id] = ids_.begin();
    evict(evi);
  }

  void evict(string& evi) {
    if (ids_.size() <= num_) return;
    auto it = ids_.rbegin();
    evi = *it;
    it_.erase(evi);
    ids_.pop_back();
  }

  void del(const string& id) {
    if (auto it = it_.find(id); it != it_.end()) {
      ids_.erase(it->second);
      it_.erase(it);
    }
  }

  // void dump() { cout << "dump:" << ids_.size() << " " << it_.size() << endl; }
};

struct Llru {
  static int update(lua_State* L);
  static int del(lua_State* L);

  static int gc(lua_State* L);
  static void meta(lua_State* L);
  static int create(lua_State* L);
};

int Llru::del(lua_State* L) {
  Lru** pp = (Lru**)luaL_checkudata(L, 1, LLRU_META);
  Lru& lru = **pp;
  size_t len;
  const char* p = luaL_checklstring(L, 2, &len);
  lru.del(string(p, len));
  return 0;
}

int Llru::update(lua_State* L) {
  Lru** pp = (Lru**)luaL_checkudata(L, 1, LLRU_META);
  Lru& lru = **pp;
  size_t len;
  const char* p = luaL_checklstring(L, 2, &len);

  string evict;
  lru.update(string(p, len), evict);
  if (evict.size() == 0) return 0;
  lua_pushlstring(L, evict.data(), evict.size());
  return 1;
}

int Llru::gc(lua_State* L) {
  Lru** pp = (Lru**)luaL_checkudata(L, 1, LLRU_META);
  delete *pp;
  return 0;
}

void Llru::meta(lua_State* L) {
  if (luaL_newmetatable(L, LLRU_META)) {
    luaL_Reg l[] = {
        {"update", update},
        {"del", del},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Llru::create(lua_State* L) {
  int num = luaL_checkinteger(L, 1);
  if (num <= 0) return 0;
  Lru* p = new Lru();
  p->num_ = num;
  Lru** pp = (Lru**)lua_newuserdata(L, sizeof(p));
  *pp = p;
  meta(L);
  return 1;
}

extern "C" {
LUAMOD_API int luaopen_llru(lua_State* L) {
  luaL_Reg funcs[] = {{"create", Llru::create}, {NULL, NULL}};

  luaL_newlib(L, funcs);
  return 1;
}
}