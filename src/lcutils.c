#include "lcurl.h"
#include "lcutils.h"
#include "lcerror.h"

int lcurl_storage_init(lua_State *L){
  lua_newtable(L);
  return luaL_ref(L, LCURL_LUA_REGISTRY);
}

void lcurl_storage_preserve_value(lua_State *L, int storage, int i){
  assert(i > 0);
  luaL_checkany(L, i);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_pushvalue(L, i); lua_pushboolean(L, 1); lua_rawset(L, -3);
  lua_pop(L, 1);
}


int lcurl_storage_preserve_slist(lua_State *L, int storage, struct curl_slist * list){
  int r;
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, 1); // list storage
  if(!lua_istable(L, -1)){
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, 1);
  }
  lua_pushlightuserdata(L, list);
  r = luaL_ref(L, -2);
  lua_pop(L, 2);
  return r;
}

struct curl_slist* lcurl_storage_remove_slist(lua_State *L, int storage, int idx){
  struct curl_slist* list;
  assert(idx != LUA_NOREF);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, 1); // list storage
  lua_rawgeti(L, -1, idx);
  list = lua_touserdata(L, -1);
  assert(list);
  luaL_unref(L, -2, idx);
  lua_pop(L, 3);
  return list;
}


void lcurl_storage_free(lua_State *L, int storage){
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, 1); // list storage
  if(lua_istable(L, -1)){
    lua_pushnil(L);
    while(lua_next(L, -2) != 0){
      struct curl_slist * list = lua_touserdata(L, -1);
      curl_slist_free_all(list);
      lua_pushvalue(L, -2); lua_pushnil(L);
      lua_rawset(L, -5);
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);
  luaL_unref(L, LCURL_LUA_REGISTRY, storage);
}

struct curl_slist* lcurl_util_array_to_slist(lua_State *L, int t){
  struct curl_slist *list = NULL;
  int i, n = lua_rawlen(L, t);

  assert(lua_type(L, t) == LUA_TTABLE);

  for(i = 1; i <= n; ++i){
    lua_rawgeti(L, t, i);
    list = curl_slist_append(list, lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  return list;
}

struct curl_slist* lcurl_util_to_slist(lua_State *L, int t){
  if(lua_type(L, t) == LUA_TTABLE){
    return lcurl_util_array_to_slist(L, t);
  }
  return 0;
}

void lcurl_util_slist_set(lua_State *L, int t, struct curl_slist* list){
  int i;
  t = lua_absindex(L, t);
  for(i = 0;list;list = list->next){
    lua_pushstring(L, list->data);
    lua_rawseti(L, t, ++i);
  }
}

void lcurl_util_slist_to_table(lua_State *L, struct curl_slist* list){
  lua_newtable(L);
  lcurl_util_slist_set(L, -1, list);
}

