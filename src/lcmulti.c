#include "lcurl.h"
#include "lceasy.h"
#include "lcmulti.h"
#include "lcerror.h"
#include "lcutils.h"
#include "lchttppost.h"

static const char *LCURL_ERROR_TAG = "LCURL_ERROR_TAG";

#define LCURL_MULTI_NAME LCURL_PREFIX" Multi"
static const char *LCURL_MULTI = LCURL_MULTI_NAME;

//{
int lcurl_multi_create(lua_State *L, int error_mode){
  lcurl_multi_t *p = lutil_newudatap(L, lcurl_multi_t, LCURL_MULTI);
  p->curl = curl_multi_init();
  if(!p->curl) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_INTERNAL_ERROR);

  return 1;
}

lcurl_multi_t *lcurl_getmulti_at(lua_State *L, int i){
  lcurl_multi_t *p = (lcurl_multi_t *)lutil_checkudatap (L, i, LCURL_MULTI);
  luaL_argcheck (L, p != NULL, 1, LCURL_MULTI_NAME" expected");
  return p;
}

static int lcurl_multi_cleanup(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  if(p->curl){
    curl_multi_cleanup(p->curl);
    p->curl = NULL;
  }

  return 0;
}


//}

static const struct luaL_Reg lcurl_multi_methods[] = {
  {"close",        lcurl_multi_cleanup          },
  {"__gc",         lcurl_multi_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_multi_opt[] = {

  {NULL, 0}
};

void lcurl_multi_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_MULTI, lcurl_multi_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_multi_opt);
}