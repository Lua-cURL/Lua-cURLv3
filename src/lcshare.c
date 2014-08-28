#include "lcurl.h"
#include "lceasy.h"
#include "lcshare.h"
#include "lcerror.h"
#include "lcutils.h"
#include "lchttppost.h"

static const char *LCURL_ERROR_TAG = "LCURL_ERROR_TAG";

#define LCURL_SHARE_NAME LCURL_PREFIX" Share"
static const char *LCURL_SHARE = LCURL_SHARE_NAME;

//{
int lcurl_share_create(lua_State *L, int error_mode){
  lcurl_share_t *p = lutil_newudatap(L, lcurl_share_t, LCURL_SHARE);
  p->curl = curl_share_init();
  p->err_mode = error_mode;
  if(!p->curl) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_SHARE, CURLSHE_NOMEM);

  return 1;
}

lcurl_share_t *lcurl_getshare_at(lua_State *L, int i){
  lcurl_share_t *p = (lcurl_share_t *)lutil_checkudatap (L, i, LCURL_SHARE);
  luaL_argcheck (L, p != NULL, 1, LCURL_SHARE_NAME" expected");
  return p;
}

static int lcurl_share_cleanup(lua_State *L){
  lcurl_share_t *p = lcurl_getshare(L);
  if(p->curl){
    curl_share_cleanup(p->curl);
    p->curl = NULL;
  }

  return 0;
}

//}

static const struct luaL_Reg lcurl_share_methods[] = {
  {"close",        lcurl_share_cleanup          },
  {"__gc",         lcurl_share_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_share_opt[] = {

  {NULL, 0}
};

void lcurl_share_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_SHARE, lcurl_share_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_share_opt);
}
