/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#include "lcurl.h"
#include "lcutils.h"
#include "lcerror.h"

#define LCURL_STORAGE_SLIST 1
#define LCURL_STORAGE_KV    2

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

static void lcurl_storage_ensure_t(lua_State *L, int t){
  lua_rawgeti(L, -1, t);
  if(!lua_istable(L, -1)){
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, t);
  }
}

int lcurl_storage_preserve_slist(lua_State *L, int storage, struct curl_slist * list){
  int r;
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lcurl_storage_ensure_t(L, LCURL_STORAGE_SLIST);
  lua_pushlightuserdata(L, list);
  r = luaL_ref(L, -2);
  lua_pop(L, 2);
  return r;
}

void lcurl_storage_preserve_iv(lua_State *L, int storage, int i, int v){
  v = lua_absindex(L, v);

  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lcurl_storage_ensure_t(L, LCURL_STORAGE_KV);
  lua_pushvalue(L, v);
  lua_rawseti(L, -2, i);
  lua_pop(L, 2);
}

void lcurl_storage_remove_i(lua_State *L, int storage, int i){
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, LCURL_STORAGE_KV);
  if(lua_istable(L, -1)){
    lua_pushnil(L);
    lua_rawseti(L, -2, i);
  }
  lua_pop(L, 2);
}

void lcurl_storage_get_i(lua_State *L, int storage, int i){
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, LCURL_STORAGE_KV);
  if(lua_istable(L, -1)){
    lua_rawgeti(L, -1, i);
    lua_remove(L, -2);
  }
  lua_remove(L, -2);
}

struct curl_slist* lcurl_storage_remove_slist(lua_State *L, int storage, int idx){
  struct curl_slist* list = NULL;
  assert(idx != LUA_NOREF);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, LCURL_STORAGE_SLIST); // list storage
  if(lua_istable(L, -1)){
    lua_rawgeti(L, -1, idx);
    list = lua_touserdata(L, -1);
    assert(list);
    luaL_unref(L, -2, idx);
    lua_pop(L, 1);
  }
  lua_pop(L, 2);
  return list;
}

int lcurl_storage_free(lua_State *L, int storage){
  lua_rawgeti(L, LCURL_LUA_REGISTRY, storage);
  lua_rawgeti(L, -1, LCURL_STORAGE_SLIST); // list storage
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
  return LUA_NOREF;
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

void lcurl_util_set_const(lua_State *L, const lcurl_const_t *reg){
  const lcurl_const_t *p;
  for(p = reg; p->name; ++p){
    lua_pushstring(L, p->name);
    lua_pushnumber(L, p->value);
    lua_settable(L, -3);
  }
}

int lcurl_set_callback(lua_State *L, lcurl_callback_t *c, int i, const char *method){
  int top = lua_gettop(L);
  i = lua_absindex(L, i);

  luaL_argcheck(L, !lua_isnoneornil(L, i), i, "no function present");
  luaL_argcheck(L, (top < (i + 2)), i + 2, "no arguments expected");

  // if(top > (i + 1)) lua_settop(L, i + 1); // this for force ignore other arguments

  assert((top == i)||(top == (i + 1)));

  if(c->ud_ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, c->ud_ref);
    c->ud_ref = LUA_NOREF;
  }

  if(c->cb_ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, c->cb_ref);
    c->cb_ref = LUA_NOREF;
  }

  if(lua_gettop(L) == (i + 1)){// function + context
    c->ud_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);

    assert(top == (2 + lua_gettop(L)));
    return 1;
  }

  assert(top == i);

  if(lua_isfunction(L, i)){ // function
    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);

    assert(top == (1 + lua_gettop(L)));
    return 1;
  }

  if(lua_isuserdata(L, i) || lua_istable(L, i)){ // object
    lua_getfield(L, i, method);

    luaL_argcheck(L, lua_isfunction(L, -1), 2, "method not found in object");

    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    c->ud_ref = luaL_ref(L, LCURL_LUA_REGISTRY);

    assert(top == (1 + lua_gettop(L)));
    return 1;
  }

  lua_pushliteral(L, "invalid object type");
  return lua_error(L);
}

int lcurl_util_push_cb(lua_State *L, lcurl_callback_t *c){
  assert(c->cb_ref != LUA_NOREF);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, c->cb_ref);
  if(c->ud_ref != LUA_NOREF){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, c->ud_ref);
    return 2;
  }
  return 1;
}

int lcurl_util_new_weak_table(lua_State*L, const char *mode){
  int top = lua_gettop(L);
  lua_newtable(L);
  lua_newtable(L);
  lua_pushstring(L, mode);
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L,-2);
  assert((top+1) == lua_gettop(L));
  return 1;
}

int lcurl_util_pcall_method(lua_State *L, const char *name, int nargs, int nresults, int errfunc){
  int obj_index = -nargs - 1;
  lua_getfield(L, obj_index, name);
  lua_insert(L, obj_index - 1);
  return lua_pcall(L, nargs + 1, nresults, errfunc);
}

static void lcurl_utils_pcall_close(lua_State *L, int obj){
  int top = lua_gettop(L);
  lua_pushvalue(L, obj);
  lcurl_util_pcall_method(L, "close", 0, 0, 0);
  lua_settop(L, top);
}

int lcurl_utils_apply_options(lua_State *L, int opt, int obj, int do_close,
  int error_mode, int error_type, int error_code
){
  int top = lua_gettop(L);
  opt = lua_absindex(L, opt);
  obj = lua_absindex(L, obj);

  lua_pushnil(L);
  while(lua_next(L, opt) != 0){
    int n;
    assert(lua_gettop(L) == (top + 2));
    
    if(lua_type(L, -2) == LUA_TNUMBER){ /* [curl.OPT_URL] = "http://localhost" */
      lua_pushvalue(L, -2);
      lua_insert(L, -2);            /*Stack : opt, obj, k, k, v */
      lua_pushliteral(L, "setopt"); /*Stack : opt, obj, k, k, v, "setopt" */
      n = 2;
    }
    else if(lua_type(L, -2) == LUA_TSTRING){ /* url = "http://localhost" */
      lua_pushliteral(L, "setopt_"); lua_pushvalue(L, -3); lua_concat(L, 2);
      /*Stack : opt, obj, k, v, "setopt_XXX" */
      n = 1;
    }
    else{
      lua_pop(L, 1);
      continue;
    }
    /*Stack : opt, obj, k,[ k,] v, `setoptXXX` */

    lua_gettable(L, obj); /* get e["settop_XXX]*/

    if(lua_isnil(L, -1)){ /* unknown option */
      if(do_close) lcurl_utils_pcall_close(L, obj);
      lua_settop(L, top);
      return lcurl_fail_ex(L, error_mode, error_type, error_code);
    }

    lua_insert(L, -n-1);       /*Stack : opt, obj, k, setoptXXX, [ k,] v       */
    lua_pushvalue(L, obj);     /*Stack : opt, obj, k, setoptXXX, [ k,] v, obj  */
    lua_insert(L, -n-1);       /*Stack : opt, obj, k, setoptXXX,  obj, [ k,] v */

    if(lua_pcall(L, n+1, 2, 0)){
      if(do_close) lcurl_utils_pcall_close(L, obj);
      return lua_error(L);
    }

    if(lua_isnil(L, -2)){
      if(do_close) lcurl_utils_pcall_close(L, obj);
      lua_settop(L, top);
      return 2;
    }

    /*Stack : opt, obj, k, ok, nil*/
    lua_pop(L, 2);
    assert(lua_gettop(L) == (top+1));
  }
  assert(lua_gettop(L) == top);
  return 0;
}

void lcurl_stack_dump (lua_State *L){
  int i = 1, top = lua_gettop(L);

  fprintf(stderr, " ----------------  Stack Dump ----------------\n" );
  while( i <= top ) {
    int t = lua_type(L, i);
    switch (t) {
      case LUA_TSTRING:
        fprintf(stderr, "%d(%d):`%s'\n", i, i - top - 1, lua_tostring(L, i));
        break;
      case LUA_TBOOLEAN:
        fprintf(stderr, "%d(%d): %s\n",  i, i - top - 1,lua_toboolean(L, i) ? "true" : "false");
        break;
      case LUA_TNUMBER:
        fprintf(stderr, "%d(%d): %g\n",  i, i - top - 1, lua_tonumber(L, i));
        break;
      default:
        lua_getglobal(L, "tostring");
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        fprintf(stderr, "%d(%d): %s(%s)\n", i, i - top - 1, lua_typename(L, t), lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    }
    i++;
  }
  fprintf(stderr, " ------------ Stack Dump Finished ------------\n" );
}

curl_socket_t lcurl_opt_os_socket(lua_State *L, int idx, curl_socket_t def) {
  if (lua_islightuserdata(L, idx))
    return (curl_socket_t)lua_touserdata(L, idx);

  return (curl_socket_t)lutil_optint64(L, idx, def);
}

void lcurl_push_os_socket(lua_State *L, curl_socket_t fd) {
#if !defined(_WIN32)
  lutil_pushint64(L, fd);
#else /*_WIN32*/
  /* Assumes that compiler can optimize constant conditions. MSVC do this. */

  /*On Lua 5.3 lua_Integer type can be represented exactly*/
#if LUA_VERSION_NUM >= 503
  if (sizeof(curl_socket_t) <= sizeof(lua_Integer)) {
    lua_pushinteger(L, (lua_Integer)fd);
    return;
  }
#endif

#if defined(LUA_NUMBER_DOUBLE) || defined(LUA_NUMBER_FLOAT)
  /*! @todo test DBL_MANT_DIG, FLT_MANT_DIG */

  if (sizeof(lua_Number) == 8) { /*we have 53 bits for integer*/
    if ((sizeof(curl_socket_t) <= 6)) {
      lua_pushnumber(L, (lua_Number)fd);
      return;
    }

    if(((UINT_PTR)fd & 0x1FFFFFFFFFFFFF) == (UINT_PTR)fd)
      lua_pushnumber(L, (lua_Number)fd);
    else
      lua_pushlightuserdata(L, (void*)fd);

    return;
  }

  if (sizeof(lua_Number) == 4) { /*we have 24 bits for integer*/
    if (((UINT_PTR)fd & 0xFFFFFF) == (UINT_PTR)fd)
      lua_pushnumber(L, (lua_Number)fd);
    else
      lua_pushlightuserdata(L, (void*)fd);
    return;
  }
#endif

  lutil_pushint64(L, fd);
  if (lcurl_opt_os_socket(L, -1, 0) != fd)
    lua_pushlightuserdata(L, (void*)fd);

#endif /*_WIN32*/
}
