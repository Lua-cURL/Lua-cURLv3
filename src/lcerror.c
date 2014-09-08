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
#include "lcerror.h"
#include <assert.h>
#include "lcutils.h"

#define LCURL_ERROR_NAME LCURL_PREFIX" Error"
static const char *LCURL_ERROR = LCURL_ERROR_NAME;

typedef struct lcurl_error_tag{
  int tp;
  int no;
}lcurl_error_t;

//{

static const char* lcurl_err_easy_mnemo(int err){
#define ERR_ENTRY(E) case CURLE_##E: return #E;

  switch (err){
    #include "lcerr_easy.h"
  }
  return "UNKNOWN";

#undef ERR_ENTRY
}

static const char* lcurl_err_multi_mnemo(int err){
#define ERR_ENTRY(E) case CURLM_##E: return #E;

  switch (err){
    #include "lcerr_multi.h"
  }
  return "UNKNOWN";

#undef ERR_ENTRY
}

static const char* lcurl_err_share_mnemo(int err){
#define ERR_ENTRY(E) case CURLSHE_##E: return #E;

  switch (err){
    #include "lcerr_share.h"
  }
  return "UNKNOWN";

#undef ERR_ENTRY
}

static const char* lcurl_err_form_mnemo(int err){
#define ERR_ENTRY(E) case CURL_FORMADD_##E: return #E;

  switch (err){
    #include "lcerr_form.h"
  }
  return "UNKNOWN";

#undef ERR_ENTRY
}

static const char* _lcurl_err_mnemo(int tp, int err){
  switch(tp){
    case LCURL_ERROR_EASY : return lcurl_err_easy_mnemo (err);
    case LCURL_ERROR_MULTI: return lcurl_err_multi_mnemo(err);
    case LCURL_ERROR_SHARE: return lcurl_err_share_mnemo(err);
    case LCURL_ERROR_FORM : return lcurl_err_form_mnemo (err);
  }
  assert(0);
  return "<UNSUPPORTED ERROR TYPE>";
}

static const char* _lcurl_err_msg(int tp, int err){
  switch(tp){
    case LCURL_ERROR_EASY : return curl_easy_strerror (err);
    case LCURL_ERROR_MULTI: return curl_multi_strerror(err);
    case LCURL_ERROR_SHARE: return curl_share_strerror(err);
    case LCURL_ERROR_FORM : return lcurl_err_form_mnemo(err);
  }
  assert(0);
  return "<UNSUPPORTED ERROR TYPE>";
}

static void _lcurl_err_pushstring(lua_State *L, int tp, int err){
  lua_pushfstring(L, "[%s] %s (%d)",
    _lcurl_err_mnemo(tp, err),
    _lcurl_err_msg(tp, err),
    err
  );
}

//}

//{

int lcurl_error_create(lua_State *L, int error_type, int no){
  lcurl_error_t *err = lutil_newudatap(L, lcurl_error_t, LCURL_ERROR);

  assert(
    (error_type == LCURL_ERROR_EASY ) ||
    (error_type == LCURL_ERROR_MULTI) ||
    (error_type == LCURL_ERROR_SHARE) ||
    (error_type == LCURL_ERROR_FORM ) ||
    0
  );

  err->tp = error_type;
  err->no = no;
  return 1;
}

static lcurl_error_t *lcurl_geterror_at(lua_State *L, int i){
  lcurl_error_t *err = (lcurl_error_t *)lutil_checkudatap (L, i, LCURL_ERROR);
  luaL_argcheck (L, err != NULL, 1, LCURL_PREFIX"error object expected");
  return err;
}

#define lcurl_geterror(L) lcurl_geterror_at((L),1)

static int lcurl_err_no(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushinteger(L, err->no);
  return 1;
}

static int lcurl_err_msg(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushstring(L, _lcurl_err_msg(err->tp, err->no));
  return 1;
}

static int lcurl_err_mnemo(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushstring(L, _lcurl_err_mnemo(err->tp, err->no));
  return 1;
}

static int lcurl_err_tostring(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  _lcurl_err_pushstring(L, err->tp, err->no);
  return 1;
}

static int lcurl_err_equal(lua_State *L){
  lcurl_error_t *lhs = lcurl_geterror_at(L, 1);
  lcurl_error_t *rhs = lcurl_geterror_at(L, 2);
  lua_pushboolean(L, ((lhs->no == rhs->no)&&(lhs->tp == rhs->tp))?1:0);
  return 1;
}

static int lcurl_err_category(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushinteger(L, err->tp);
  return 1;
}

//}

//{

int lcurl_fail_ex(lua_State *L, int mode, int error_type, int code){
  if(mode == LCURL_ERROR_RETURN){
    lua_pushnil(L);
    lcurl_error_create(L, error_type, code);
    return 2;
  }

#if LUA_VERSION_NUM >= 502 // lua 5.2
  lcurl_error_create(L, error_type, code);
#else
  _lcurl_err_pushstring(L, error_type, code);
#endif

  assert(LCURL_ERROR_RAISE == mode);

  return lua_error(L);
}

int lcurl_fail(lua_State *L, int error_type, int code){
  return lcurl_fail_ex(L, LCURL_ERROR_RETURN, error_type, code);
}

//}

int lcurl_error_new(lua_State *L){
  int tp = luaL_checkint(L, 1);
  int no = luaL_checkint(L, 2);

  //! @todo checks error type value

  lcurl_error_create(L, tp, no);
  return 1;
}

static const struct luaL_Reg lcurl_err_methods[] = {
  {"no",              lcurl_err_no               },
  {"msg",             lcurl_err_msg              },
  {"name",            lcurl_err_mnemo            },
  {"mnemo",           lcurl_err_mnemo            },
  {"cat",             lcurl_err_category         },
  {"category",        lcurl_err_category         },
  {"__tostring",      lcurl_err_tostring         },
  {"__eq",            lcurl_err_equal            },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_error_codes[] = {

#define ERR_ENTRY(N) { "E_"#N, CURLE_##N },
#include "lcerr_easy.h"
#undef ERR_ENTRY

#define ERR_ENTRY(N) { "E_MULTI_"#N, CURLM_##N },
#include "lcerr_multi.h"
#undef ERR_ENTRY

#define ERR_ENTRY(N) { "E_SHARE_"#N, CURLSHE_##N },
#include "lcerr_share.h"
#undef ERR_ENTRY

#define ERR_ENTRY(N) { "E_FORM_"#N, CURL_FORMADD_##N },
#include "lcerr_form.h"
#undef ERR_ENTRY

  {NULL, 0}
};

static const lcurl_const_t lcurl_error_category[] = {
  {"ERROR_CURL",  LCURL_ERROR_CURL},
  {"ERROR_EASY",  LCURL_ERROR_EASY},
  {"ERROR_MULTI", LCURL_ERROR_MULTI},
  {"ERROR_SHARE", LCURL_ERROR_SHARE},
  {"ERROR_FORM",  LCURL_ERROR_FORM},

  {NULL, 0}
};

void lcurl_error_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_ERROR, lcurl_err_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_error_codes);
  lcurl_util_set_const(L, lcurl_error_category);
}
