extern "C" {
#include <fnmatch.h>

#include "lauxlib.h"
}
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "leveldb/filter_policy.h"

struct Db {
  leveldb::DB* db_ = nullptr;
  leveldb::Options options_;
  ~Db() {
    if (db_) delete db_;
    if (options_.block_cache) delete options_.block_cache;
    if (options_.filter_policy) delete options_.filter_policy;
  }
  bool open(const string& name, int64_t cache_size) {
    options_.create_if_missing = true;
    options_.compression = leveldb::kZstdCompression;
    options_.write_buffer_size = 8 * 1024 * 1024;
    options_.max_file_size = 4 * 1024 * 1024;
    options_.block_size = 16 * 1024;

    if (cache_size > 1024 * 1024) {
      options_.block_cache = leveldb::NewLRUCache(cache_size);
      options_.filter_policy = leveldb::NewBloomFilterPolicy(10);
    }
    leveldb::Status status = leveldb::DB::Open(options_, name, &db_);
    if (!status.ok()) {
      return false;
    }
    return true;
  }
};

static int compact(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);
  p->db_->CompactRange(nullptr, nullptr);
  return 0;
}

static int put(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);

  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  size_t lv;
  const char* pv = luaL_checklstring(L, 3, &lv);
  p->db_->Put(leveldb::WriteOptions(), {pk, lk}, {pv, lv});
  return 0;
}

static int del(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);

  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  p->db_->Delete(leveldb::WriteOptions(), {pk, lk});
  return 0;
}

static int get(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);

  size_t lk;
  const char* pk = luaL_checklstring(L, 2, &lk);
  string val;
  auto s = p->db_->Get(leveldb::ReadOptions(), {pk, lk}, &val);
  if (!s.ok()) return 0;
  lua_pushlstring(L, val.data(), val.size());
  return 1;
}

static int scan(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);

  size_t ls;
  const char* ps = luaL_checklstring(L, 2, &ls);
  string start(ps, ls);
  size_t le;
  const char* pe = luaL_checklstring(L, 3, &le);
  string end(pe, le);
  size_t lp;
  const char* pp = luaL_checklstring(L, 4, &lp);
  string patt(pp, lp);
  int count = luaL_checkinteger(L, 5);

  int c = 0;
  lua_createtable(L, 2 * count + 2, 0);
  leveldb::Iterator* it = p->db_->NewIterator(leveldb::ReadOptions());
  for (it->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
    if (c > count * 2) break;
    string key = it->key().ToString();
    string val = it->value().ToString();
    if (0 != fnmatch(patt.data(), key.data(), 0)) continue;
    lua_pushlstring(L, key.data(), key.size());
    lua_rawseti(L, -2, ++c);
    lua_pushlstring(L, val.data(), val.size());
    lua_rawseti(L, -2, ++c);
  }
  delete it;
  return 1;
}

static int create(lua_State* L) {
  size_t ln;
  const char* pn = luaL_checklstring(L, 1, &ln);
  string name(pn, ln);
  int64_t cache_size = lua_tointeger(L, 2);

  Db* p = new Db();
  bool ok = p->open(name, cache_size);
  if (!ok) {
    delete p;
    return luaL_error(L, "leveldb create err");
  }
  lua_pushlightuserdata(L, p);
  return 1;
}

static int release(lua_State* L) {
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  Db* p = (Db*)lua_touserdata(L, 1);
  delete p;
  return 0;
}

extern "C" {
LUAMOD_API int luaopen_lleveldb(lua_State* L) {
  luaL_Reg l[] = {{"create", create},   {"release", release},
                  {"compact", compact}, {"scan", scan},
                  {"get", get},         {"put", put},
                  {"del", del},         {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}
}