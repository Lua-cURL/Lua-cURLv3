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
  p->err_mode = error_mode;
  lcurl_util_new_weak_table(L, "v");
  p->h_ref = luaL_ref(L, LCURL_LUA_REGISTRY);

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

  if(p->h_ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, p->h_ref);
    p->h_ref = LUA_NOREF;
  }

  return 0;
}

static int lcurl_multi_add_handle(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  lcurl_easy_t  *e = lcurl_geteasy_at(L, 2);
  CURLMcode code = curl_multi_add_handle(p->curl, e->curl);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
  lua_pushvalue(L, 2);
  lua_rawsetp(L, -2, e->curl);
  lua_settop(L, 1);
  return 1;
}

static int lcurl_multi_remove_handle(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  lcurl_easy_t  *e = lcurl_geteasy_at(L, 2);
  CURLMcode code = curl_multi_remove_handle(p->curl, e->curl);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
  lua_pushnil(L);
  lua_rawsetp(L, -2, e->curl);
  lua_settop(L, 1);
  return 1;
}

static int lcurl_multi_perform(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int running_handles = 0;
  CURLMcode code = curl_multi_perform(p->curl, &running_handles);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushnumber(L, running_handles);
  return 1;
}

static int lcurl_multi_info_read(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int msgs_in_queue = 0;
  CURLMsg *msg = curl_multi_info_read(p->curl, &msgs_in_queue);
  lcurl_easy_t *e;
  if(!msg){
    lua_pushnumber(L, msgs_in_queue);
    return 1;
  }

  if(msg->msg == CURLMSG_DONE){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_rawgetp(L, -1, msg->easy_handle);
    e = lcurl_geteasy_at(L, -1);
    if(msg->data.result == CURLE_OK){
      lua_pushboolean(L, 1);
      return 2;
    }
    return 1 + lcurl_fail_ex(L, LCURL_ERROR_RETURN, LCURL_ERROR_EASY, msg->data.result);
  }

  // @todo handle unknown message
  lua_pushboolean(L, 0);
  return 1;
}

//{ OPTIONS
static int lcurl_opt_set_long_(lua_State *L, int opt){
  lcurl_multi_t *p = lcurl_getmulti(L);
  long val; CURLMcode code;

  if(lua_isboolean(L, 2)) val = lua_toboolean(L, 2);
  else{
    luaL_argcheck(L, lua_type(L, 2) == LUA_TNUMBER, 2, "number or boolean expected");
    val = luaL_checklong(L, 2);
  }
  
  code = curl_multi_setopt(p->curl, opt, val);
  if(code != CURLM_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_settop(L, 1);
  return 1;
}

static int lcurl_opt_set_string_array_(lua_State *L, int opt){
  lcurl_multi_t *p = lcurl_getmulti(L);
  CURLMcode code;
  int n;
  luaL_argcheck(L, lua_type(L, 2) == LUA_TTABLE, 2, "array expected");
  n = lua_rawlen(L, 2);
  if(n == 0){
    char *val[] = {NULL};
    code = curl_multi_setopt(p->curl, opt, val);
  }
  else{
    int i;
    char const* *val = malloc(sizeof(char*) * (n + 1));
    if(!*val){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_OUT_OF_MEMORY);
    }
    for(i = 1; i <= n; ++i){
      lua_rawgeti(L, 2, i);
      val[i-1] = lua_tostring(L, -1);
      lua_pop(L, 1);
    }
    val[n] = NULL;
    code = curl_multi_setopt(p->curl, opt, val);
    free((void*)val);
  }

  if(code != CURLM_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }

  lua_settop(L, 1);
  return 1;
}

#define LCURL_LNG_OPT(N, S) static int lcurl_multi_set_##N(lua_State *L){\
  return lcurl_opt_set_long_(L, CURLMOPT_##N);\
}

#define LCURL_STR_ARR_OPT(N, S) static int lcurl_multi_set_##N(lua_State *L){\
  return lcurl_opt_set_string_array_(L, CURLMOPT_##N);\
}

#define OPT_ENTRY(L, N, T, S) LCURL_##T##_OPT(N, S)

#include "lcoptmulti.h"

#undef OPT_ENTRY
#undef LCURL_LNG_OPT
#undef LCURL_STR_ARR_OPT

static int lcurl_multi_setopt(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S) case CURLMOPT_##N: return lcurl_multi_set_##N(L);
  switch(opt){
    #include "lcoptmulti.h"
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_UNKNOWN_OPTION);
}

//}

//}

static const struct luaL_Reg lcurl_multi_methods[] = {
  {"add_handle",       lcurl_multi_add_handle       },
  {"remove_handle",    lcurl_multi_remove_handle    },
  {"perform",          lcurl_multi_perform          },
  {"info_read",        lcurl_multi_info_read        },
  {"setopt",           lcurl_multi_setopt           },

#define OPT_ENTRY(L, N, T, S) { "setopt_"#L, lcurl_multi_set_##N },
  #include "lcoptmulti.h"
#undef OPT_ENTRY

  {"close",            lcurl_multi_cleanup          },
  {"__gc",             lcurl_multi_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_multi_opt[] = {
#define OPT_ENTRY(L, N, T, S) { "OPT_MULTI_"#N, CURLMOPT_##N },
#include "lcoptmulti.h"
#undef OPT_ENTRY

  {NULL, 0}
};

void lcurl_multi_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_MULTI, lcurl_multi_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_multi_opt);
}