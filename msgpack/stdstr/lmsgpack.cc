extern "C" {
#include "lauxlib.h"
}
#include <iostream>

#include "msgpack.h"
using namespace std;

struct Pack {
  lua_State* L;
  Msgpack::Pack pack_;
  uint32_t dep_;

  void pack(int index);
  uint32_t table_len(int index);
  void pack_table(int index);
};
uint32_t Pack::table_len(int index) {
  uint32_t i = 0;
  lua_pushnil(L);
  while (lua_next(L, index) != 0) {
    ++i;
    lua_pop(L, 1);
  }
  return i;
}
void Pack::pack_table(int index) {
  ++dep_;
  lua_checkstack(L, 2);
  if (dep_ > 10) {
    luaL_error(L, "pack dep err");
    return;
  }
  if (index < 0) index = lua_gettop(L) + index + 1;
  int32_t rawlen = lua_rawlen(L, index);
  int32_t tablen = table_len(index);
  bool ismap;
  if (rawlen == tablen) {
    pack_.pack_arr_head(tablen);
    ismap = false;
  } else {
    pack_.pack_map_head(tablen);
    ismap = true;
  }

  lua_pushnil(L);
  while (lua_next(L, index) != 0) {
    if (!ismap) {
      pack(-1);
    } else {
      pack(-2);
      pack(-1);
    }
    lua_pop(L, 1);
  }
  --dep_;
}
