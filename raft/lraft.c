#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "lauxlib.h"
#include "raft/raft.h"

typedef struct Udata {
  lua_State* L;
} Udata;

static const char* RAFT = "RAFT";

static lua_State* push_func(Udata* pu, int idx) {
  lua_State* L = pu->L;
  lua_getiuservalue(L, 1, 1);
  lua_rawgeti(L, -1, idx);
  return L;
}

static void* get_raft(lua_State* L) {
  void** pp = (void**)luaL_checkudata(L, 1, RAFT);
  void* p = *pp;
  Udata* pu = raft_get_udata(p);
  pu->L = L;
  return p;
}

static int get_leader(lua_State* L) {
  void* raft = get_raft(L);
  void* leader = raft_get_current_leader_node(raft);
  if (!leader) return 0;
  int id = raft_node_get_id(leader);
  lua_pushlightuserdata(L, leader);
  lua_pushinteger(L, id);
  return 2;
}

static int entry_res_commited(lua_State* L) {
  void* raft = get_raft(L);
  size_t lb;
  const char* pb = luaL_checklstring(L, 2, &lb);
  msg_entry_response_t* m = (msg_entry_response_t*)pb;
  int e = raft_msg_entry_response_committed(raft, m);
  lua_pushinteger(L, e);
  return 1;
}

static int recv_entry(lua_State* L) {
  void* raft = get_raft(L);
  msg_entry_t entry;
  size_t lb;
  entry.data.buf = (char*)luaL_checklstring(L, 2, &lb);
  entry.data.len = lb;
  msg_entry_response_t res;
  int e = raft_recv_entry(raft, &entry, &res);
  if (e != 0) return 0;
  lua_pushlstring(L, (char*)&res, sizeof(res));
  return 1;
}

static int tick(lua_State* L) {
  void* raft = get_raft(L);
  raft_periodic(raft, 1000);
  return 0;
}

static int requestvote(lua_State* L) {
  void* raft = get_raft(L);
  void* node = lua_touserdata(L, 2);
  size_t lb;
  const char* pb = luaL_checklstring(L, 3, &lb);
  msg_requestvote_t* m = (msg_requestvote_t*)pb;
  msg_requestvote_response_t res;
  int e = raft_recv_requestvote(raft, node, m, &res);
  if (e != 0) return 0;
  lua_pushlstring(L, (char*)&res, sizeof(res));
  return 1;
}

static int requestvote_res(lua_State* L) {
  void* raft = get_raft(L);
  void* node = lua_touserdata(L, 2);
  size_t lb;
  const char* pb = luaL_checklstring(L, 3, &lb);
  msg_requestvote_response_t* m = (msg_requestvote_response_t*)pb;
  int e = raft_recv_requestvote_response(raft, node, m);
  return 0;
}

static int appendentries(lua_State* L) {
  void* raft = get_raft(L);
  void* node = lua_touserdata(L, 2);
  size_t lh;
  const char* ph = luaL_checklstring(L, 3, &lh);
  msg_appendentries_t* m = (msg_appendentries_t*)ph;
  if (!lua_isnil(L, 4)) {
    size_t le;
    const char* pe = luaL_checklstring(L, 4, &le);
    m->entries = (msg_entry_t*)pe;
    msg_entry_t* entries = m->entries;
    size_t lb;
    const char* pb = luaL_checklstring(L, 5, &lb);
    entries->data.buf = (void*)pb;
    entries->data.len = lb;
  }
  msg_appendentries_response_t res;
  int e = raft_recv_appendentries(raft, node, m, &res);
  if (0 != e) return 0;
  lua_pushlstring(L, (char*)&res, sizeof(res));
  return 1;
}

static int appendentries_res(lua_State* L) {
  void* raft = get_raft(L);
  void* node = lua_touserdata(L, 2);
  size_t lb;
  const char* pb = luaL_checklstring(L, 3, &lb);
  msg_appendentries_response_t* m = (msg_appendentries_response_t*)pb;
  raft_recv_appendentries_response(raft, node, m);
  return 0;
}

static int add_node(lua_State* L) {
  void* raft = get_raft(L);
  int id = luaL_checkinteger(L, 2);
  bool self = lua_toboolean(L, 3);
  void* node = raft_add_node(raft, NULL, id, self);
  lua_pushlightuserdata(L, node);
  return 1;
}

static int __send_requestvote(raft_server_t* raft, void* user_data,
                              raft_node_t* node, msg_requestvote_t* m) {
  lua_State* L = push_func(user_data, 1);
  int id = raft_node_get_id(node);
  lua_pushinteger(L, id);
  lua_pushlstring(L, (char*)m, sizeof(msg_requestvote_t));
  lua_call(L, 2, 0);
  return 0;
}

static int __send_appendentries(raft_server_t* raft, void* user_data,
                                raft_node_t* node, msg_appendentries_t* m) {
  lua_State* L = push_func(user_data, 2);
  int id = raft_node_get_id(node);
  lua_pushinteger(L, id);
  lua_pushlstring(L, (char*)m, sizeof(msg_appendentries_t));
  if (m->n_entries <= 0) {
    lua_call(L, 2, 0);
    return 0;
  }
  msg_entry_t* entry = m->entries;
  lua_pushlstring(L, (char*)entry, sizeof(msg_entry_t));
  lua_pushlstring(L, entry->data.buf, entry->data.len);
  lua_call(L, 4, 0);
  return 0;
}

static int __applylog(raft_server_t* raft, void* user_data, raft_entry_t* entry,
                      raft_index_t entry_idx) {
  lua_State* L = push_func(user_data, 3);
  lua_pushinteger(L, entry_idx);
  lua_pushlstring(L, entry->data.buf, entry->data.len);
  lua_call(L, 2, 0);
  return 0;
}

static int __persist_vote(raft_server_t* raft, void* user_data,
                          raft_node_id_t vote) {
  lua_State* L = push_func(user_data, 4);
  lua_pushinteger(L, vote);
  lua_call(L, 1, 0);
  return 0;
}

static int __persist_term(raft_server_t* raft, void* user_data,
                          raft_term_t term, raft_node_id_t vote) {
  lua_State* L = push_func(user_data, 5);
  lua_pushinteger(L, term);
  lua_pushinteger(L, vote);
  lua_call(L, 2, 0);
  return 0;
}

static int __log_offer(raft_server_t* raft, void* user_data,
                       raft_entry_t* entry, raft_index_t entry_idx) {
  lua_State* L = push_func(user_data, 6);
  lua_pushinteger(L, entry_idx);
  lua_pushlstring(L, entry->data.buf, entry->data.len);
  lua_call(L, 2, 0);
  return 0;
}

static int __log_pop(raft_server_t* raft, void* user_data, raft_entry_t* entry,
                     raft_index_t entry_idx) {
  lua_State* L = push_func(user_data, 7);
  lua_pushinteger(L, entry_idx);
  lua_pushlstring(L, entry->data.buf, entry->data.len);
  lua_call(L, 2, 0);
  return 0;
}

static int gc(lua_State* L) {
  void* raft = get_raft(L);
  Udata* pu = raft_get_udata(raft);
  free(pu);
  raft_free(raft);
  return 0;
}

static int create(lua_State* L) {
  raft_cbs_t raft_cbs = {
      .send_requestvote = __send_requestvote,
      .send_appendentries = __send_appendentries,
      .applylog = __applylog,
      .persist_vote = __persist_vote,
      .persist_term = __persist_term,
      .log_offer = __log_offer,
      .log_pop = __log_pop,
  };
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 1);
  void* p = raft_new();
  Udata* pu = malloc(sizeof(Udata));
  pu->L = L;
  raft_set_callbacks(p, &raft_cbs, pu);

  void** pp = (void**)lua_newuserdatauv(L, sizeof(p), 1);
  lua_pushvalue(L, 1);
  lua_setiuservalue(L, -2, 1);
  *pp = p;
  if (luaL_newmetatable(L, RAFT)) {
    luaL_Reg l[] = {{"add_node", add_node},
                    {"tick", tick},
                    {"get_leader", get_leader},
                    {"requestvote", requestvote},
                    {"requestvote_res", requestvote_res},
                    {"appendentries", appendentries},
                    {"appendentries_res", appendentries_res},
                    {"recv_entry", recv_entry},
                    {"entry_res_commited", entry_res_commited},
                    {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  return 1;
}

LUAMOD_API int luaopen_lraft(lua_State* L) {
  luaL_Reg l[] = {{"create", create}, {NULL, NULL}};
  luaL_newlib(L, l);
  return 1;
}