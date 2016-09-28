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
#include "lcshare.h"
#include "lcerror.h"
#include "lcutils.h"
#include "lchttppost.h"

#define LCURL_SHARE_NAME LCURL_PREFIX" Share"
static const char *LCURL_SHARE = LCURL_SHARE_NAME;

//{
int lcurl_share_create(lua_State *L, int error_mode){
  lcurl_share_t *p;

  lua_settop(L, 1);

  p = lutil_newudatap(L, lcurl_share_t, LCURL_SHARE);
  p->curl = curl_share_init();
  p->err_mode = error_mode;
  if(!p->curl) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_SHARE, CURLSHE_NOMEM);

  if(lua_type(L, 1) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 1, 2, 1, p->err_mode, LCURL_ERROR_SHARE, CURLSHE_BAD_OPTION);
    if(ret) return ret;
    assert(lua_gettop(L) == 2);
  }

  return 1;
}

lcurl_share_t *lcurl_getshare_at(lua_State *L, int i){
  lcurl_share_t *p = (lcurl_share_t *)lutil_checkudatap (L, i, LCURL_SHARE);
  luaL_argcheck (L, p != NULL, 1, LCURL_SHARE_NAME" object expected");
  return p;
}

static int lcurl_easy_to_s(lua_State *L){
  lcurl_share_t *p = (lcurl_share_t *)lutil_checkudatap (L, 1, LCURL_SHARE);
  lua_pushfstring(L, LCURL_SHARE_NAME" (%p)", (void*)p);
  return 1;
}

static int lcurl_share_cleanup(lua_State *L){
  lcurl_share_t *p = lcurl_getshare(L);
  if(p->curl){
    curl_share_cleanup(p->curl);
    p->curl = NULL;
  }

  return 0;
}

//{ OPTIONS

static int lcurl_opt_set_long_(lua_State *L, int opt){
  lcurl_share_t *p = lcurl_getshare(L);
  long val; CURLSHcode code;

  if(lua_isboolean(L, 2)) val = lua_toboolean(L, 2);
  else{
    luaL_argcheck(L, lua_type(L, 2) == LUA_TNUMBER, 2, "number or boolean expected");
    val = luaL_checklong(L, 2);
  }

  code = curl_share_setopt(p->curl, opt, val);
  if(code != CURLSHE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_SHARE, code);
  }
  lua_settop(L, 1);
  return 1;
}

#define LCURL_LNG_OPT(N, S) static int lcurl_share_set_##N(lua_State *L){\
  return lcurl_opt_set_long_(L, CURLSHOPT_##N);\
}

#define OPT_ENTRY(L, N, T, S) LCURL_##T##_OPT(N, S)

#include "lcoptshare.h"

#undef OPT_ENTRY
#undef LCURL_LNG_OPT

//}

static int lcurl_share_setopt(lua_State *L){
  lcurl_share_t *p = lcurl_getshare(L);
  int opt;
  
  luaL_checkany(L, 2);
  if(lua_type(L, 2) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 2, 1, 0, p->err_mode, LCURL_ERROR_SHARE, CURLSHE_BAD_OPTION);
    if(ret) return ret;
    lua_settop(L, 1);
    return 1;
  }

  opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S) case CURLSHOPT_##N: return lcurl_share_set_##N(L);
  switch(opt){
    #include "lcoptshare.h"
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_SHARE, CURLSHE_BAD_OPTION);
}

//}

static const struct luaL_Reg lcurl_share_methods[] = {
  { "__tostring",  lcurl_easy_to_s              },
  {"setopt",       lcurl_share_setopt           },

#define OPT_ENTRY(L, N, T, S) { "setopt_"#L, lcurl_share_set_##N },
  #include "lcoptshare.h"
#undef OPT_ENTRY

  {"close",        lcurl_share_cleanup          },
  {"__gc",         lcurl_share_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_share_opt[] = {

#define OPT_ENTRY(L, N, T, S) { "OPT_SHARE_"#N, CURLSHOPT_##N },
#define FLG_ENTRY(N) { #N, CURL_##N },
#  include "lcoptshare.h"
#undef OPT_ENTRY
#undef FLG_ENTRY

  {NULL, 0}
};

void lcurl_share_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_SHARE, lcurl_share_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_share_opt);
}
