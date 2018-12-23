/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#include "lcurlapi.h"
#include "lcurl.h"
#include "lcerror.h"
#include "lcutils.h"
#include <memory.h>

#define LCURL_URL_NAME LCURL_PREFIX" URL"
static const char *LCURL_URL = LCURL_URL_NAME;

#if LCURL_CURL_VER_GE(7,62,0)

#define lcurl_geturl(L) lcurl_geturl_at(L, 1)

int lcurl_url_create(lua_State *L, int error_mode){
  lcurl_url_t *p;

  p = lutil_newudatap(L, lcurl_url_t, LCURL_URL);

  p->url = curl_url();
  if(!p->url) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_URL, CURLUE_OUT_OF_MEMORY);

  p->err_mode = error_mode;

  if (lua_gettop(L) > 1) {
    const char *url = luaL_checkstring(L, 1);
    unsigned int flags = 0;
    CURLUcode code;

    if (lua_gettop(L) > 2) {
      flags = (unsigned int)lutil_optint64(L, 2, 0);
    }

    code = curl_url_set(p->url, CURLUPART_URL, url, flags);
    if (code != CURLUE_OK) {
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_URL, code);
    }
  }

  return 1;
}

lcurl_url_t *lcurl_geturl_at(lua_State *L, int i){
  lcurl_url_t *p = (lcurl_url_t *)lutil_checkudatap (L, i, LCURL_URL);
  luaL_argcheck (L, p != NULL, 1, LCURL_URL_NAME" object expected");
  return p;
}

static int lcurl_url_cleanup(lua_State *L){
  lcurl_url_t *p = lcurl_geturl(L);

  if (p->url){
    curl_url_cleanup(p->url);
    p->url = NULL;
  }

  return 0;
}

static int lcurl_url_dup(lua_State *L) {
  lcurl_url_t *r = lcurl_geturl(L);
  lcurl_url_t *p = lutil_newudatap(L, lcurl_url_t, LCURL_URL);

  p->url = curl_url_dup(r->url);
  if (!p->url) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_URL, CURLUE_OUT_OF_MEMORY);

  p->err_mode = r->err_mode;

  return 1;
}

static int lcurl_url_set(lua_State *L, CURLUPart what){
  lcurl_url_t *p = lcurl_geturl(L);
  CURLUcode code;
  const char *part;
  unsigned int flags = 0;
  
  luaL_argcheck(L, lua_type(L, 2) == LUA_TSTRING || lutil_is_null(L, 2), 2, "string expected");

  part = lua_tostring(L, 2);
  flags = (unsigned int)lutil_optint64(L, 3, 0);

  code = curl_url_set(p->url, what, part, flags);
  if (code != CURLUE_OK) {
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_URL, code);
  }

  lua_settop(L, 1);
  return 1;
}

static int lcurl_url_get(lua_State *L, CURLUPart what, CURLUcode empty) {
  lcurl_url_t *p = lcurl_geturl(L);
  CURLUcode code;
  char *part = NULL;
  unsigned int flags = 0;

  flags = (unsigned int)lutil_optint64(L, 2, 0);

  code = curl_url_get(p->url, what, &part, flags);
  if (code != CURLUE_OK) {
    if (part) {
      curl_free(part);
      part = NULL;
    }

    if (code != empty) {
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_URL, code);
    }
  }

  if (part == NULL) {
    lutil_push_null(L);
  }
  else {
    lua_pushstring(L, part);
    curl_free(part);
  }

  return 1;
}

static int lcurl_url_to_s(lua_State *L) {
  lcurl_url_t *p = lcurl_geturl(L);
  char *part = NULL;

  CURLUcode code = curl_url_get(p->url, CURLUPART_URL, &part, 0);

  if (code != CURLUE_OK) {
    if (part) {
      curl_free(part);
    }

    return lcurl_fail_ex(L, LCURL_ERROR_RAISE, LCURL_ERROR_URL, code);
  }

  if (part == NULL) {
    lua_pushliteral(L, "");
  }
  else {
    lua_pushstring(L, part);
    curl_free(part);
  }

  return 1;
}

#define ENTRY_PART(N, S, E) static int lcurl_url_set_##N(lua_State *L){\
  return lcurl_url_set(L, CURL##S);\
}
#define ENTRY_FLAG(S)

#include "lcopturl.h"

#undef ENTRY_PART
#undef ENTRY_FLAG

#define ENTRY_PART(N, S, E) static int lcurl_url_get_##N(lua_State *L){\
  return lcurl_url_get(L, CURL##S, E);\
}
#define ENTRY_FLAG(S)

#include "lcopturl.h"

#undef ENTRY_PART
#undef ENTRY_FLAG

static const struct luaL_Reg lcurl_url_methods[] = {
  #define ENTRY_PART(N, S, E) { "set_"#N, lcurl_url_set_##N },
  #define ENTRY_FLAG(S)
  #include "lcopturl.h"
  #undef ENTRY_PART
  #undef ENTRY_FLAG

  #define ENTRY_PART(N, S, E) { "get_"#N, lcurl_url_get_##N },
  #define ENTRY_FLAG(S)
  #include "lcopturl.h"
  #undef ENTRY_PART
  #undef ENTRY_FLAG

  { "dup",        lcurl_url_dup     },
  { "cleanup",    lcurl_url_cleanup },
  { "__gc",       lcurl_url_cleanup },
  { "__tostring", lcurl_url_to_s    },

  { NULL,NULL }
};

static const lcurl_const_t lcurl_url_opt[] = {
  #define ENTRY_PART(N, S, E) { #S, CURL##S },
  #define ENTRY_FLAG(S) { "U_"#S, CURLU_##S },
  #include "lcopturl.h"
  #undef ENTRY_PART
  #undef ENTRY_FLAG
  {NULL, 0}
};
#endif

void lcurl_url_initlib(lua_State *L, int nup){
#if LCURL_CURL_VER_GE(7,62,0)
  if(!lutil_createmetap(L, LCURL_URL, lcurl_url_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_url_opt);
#else
  lua_pop(L, nup);
#endif
}
