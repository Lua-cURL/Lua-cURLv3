#if !defined(_WINDOWS) && !defined(_WIN32)
#  define LCURL_WINDOWS
#endif

#ifdef LCURL_WINDOWS
#  include <sys/select.h>
#else
#  include <winsock2.h>
#endif

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
  p->L = L;
  p->err_mode = error_mode;
  lcurl_util_new_weak_table(L, "v");
  p->h_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
  p->tm.cb_ref = p->tm.ud_ref = LUA_NOREF;
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
  luaL_unref(L, LCURL_LUA_REGISTRY, p->tm.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->tm.ud_ref);
  p->tm.cb_ref = p->tm.ud_ref = LUA_NOREF;
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

static int lcurl_multi_wait(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  CURLMcode code;
  int maxfd; long ms;

  if(lua_isnoneornil(L, 2)){
    code = curl_multi_timeout(p->curl, &ms);
    if(code != CURLM_OK){
      lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
    }
  }
  else{
    ms = luaL_checklong(L, 2);
  }

  if(ms < 0){
    /* if libcurl returns a -1 timeout here, it just means that libcurl 
       currently has no stored timeout value. You must not wait too long
       (more than a few seconds perhaps) before you call 
       curl_multi_perform() again.
     */
    ms = 1000;
  }

#if LCURL_CURL_VER_GE(7,28,0)
  //! @todo supports extra_fds
  code = curl_multi_wait(p->curl, 0, 0, ms, &maxfd);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushnumber(L, maxfd);
  return 1;
#else
  {
  fd_set fdread, fdwrite, fdexcep;

  FD_ZERO(&fdread);
  FD_ZERO(&fdwrite);
  FD_ZERO(&fdexcep);

  code = curl_multi_fdset(p->curl, &fdread, &fdwrite, &fdexcep, &maxfd);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }

  //if(maxfd > 0)
  {
    struct timeval tv;
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;

    maxfd = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &tv);
    if(maxfd < 0){
      //! @fixme return error
    }
  }

  lua_pushnumber(L, maxfd);
  return 1;
  }
#endif
}

static int lcurl_multi_timeout(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  long n;
  CURLMcode code = curl_multi_timeout(p->curl, &n);
  if(code != CURLM_OK){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushnumber(L, n);
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

//}

//{ CallBack

static int lcurl_multi_set_callback(lua_State *L, 
  lcurl_multi_t *p, lcurl_callback_t *c,
  int OPT_CB, int OPT_UD,
  const char *method, void *func
)
{
  if(c->ud_ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, c->ud_ref);
    c->ud_ref = LUA_NOREF;
  }

  if(c->cb_ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, c->cb_ref);
    c->cb_ref = LUA_NOREF;
  }

  if(lua_gettop(L) >= 3){// function + context
    lua_settop(L, 3);
    luaL_argcheck(L, !lua_isnil(L, 2), 2, "no function present");
    c->ud_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);

    curl_multi_setopt(p->curl, OPT_UD, p);
    curl_multi_setopt(p->curl, OPT_CB, func);

    assert(1 == lua_gettop(L));
    return 1;
  }

  lua_settop(L, 2);

  if(lua_isnoneornil(L, 2)){
    lua_pop(L, 1);
    assert(1 == lua_gettop(L));

    curl_multi_setopt(p->curl, OPT_UD, 0);
    curl_multi_setopt(p->curl, OPT_CB, 0);

    return 1;
  }

  if(lua_isfunction(L, 2)){
    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    assert(1 == lua_gettop(L));

    curl_multi_setopt(p->curl, OPT_UD, p);
    curl_multi_setopt(p->curl, OPT_CB, func);
    return 1;
  }

  if(lua_isuserdata(L, 2) || lua_istable(L, 2)){
    lua_getfield(L, 2, method);
    luaL_argcheck(L, lua_isfunction(L, -1), 2, "method not found in object");
    c->cb_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    c->ud_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    curl_multi_setopt(p->curl, OPT_UD, p);
    curl_multi_setopt(p->curl, OPT_CB, func);
    assert(1 == lua_gettop(L));
    return 1;
  }

  lua_pushliteral(L, "invalid object type");
  return lua_error(L);
}

//{Timer

int lcurl_multi_timer_callback(CURLM *multi, long ms, void *arg){
  lcurl_multi_t *p = arg;
  lua_State *L = p->L;

  int ret = 0;
  int top = lua_gettop(L);
  int n   = lcurl_util_push_cb(L, &p->tm);

  lua_pushnumber(L, ms);
  if(lua_pcall(L, n, LUA_MULTRET, 0)){
    assert(lua_gettop(L) >= top);
    lua_settop(L, top); //! @todo 
    // lua_pushlightuserdata(L, (void*)LCURL_ERROR_TAG);
    // lua_insert(L, top+1);
    return -1;
  }

  if(lua_gettop(L) > top){
    if(lua_isnil(L, top + 1)){
      lua_settop(L, top);
      return -1;
    }
    
    if(lua_isboolean(L, top + 1))
      ret = lua_toboolean(L, top + 1)?0:-1;
    else ret = lua_tointeger(L, top + 1);
  }

  lua_settop(L, top);
  return ret;
}


static int lcurl_multi_set_TIMERFUNCTION(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  return lcurl_multi_set_callback(L, p, &p->tm,
    CURLMOPT_TIMERFUNCTION, CURLMOPT_TIMERDATA,
    "timer", lcurl_multi_timer_callback
  );
}

//}

//}

static int lcurl_multi_setopt(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S) case CURLMOPT_##N: return lcurl_multi_set_##N(L);
  switch(opt){
    #include "lcoptmulti.h"
    OPT_ENTRY(timerfunction, TIMERFUNCTION, TTT, 0)
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_UNKNOWN_OPTION);
}

//}

static const struct luaL_Reg lcurl_multi_methods[] = {
  {"add_handle",       lcurl_multi_add_handle       },
  {"remove_handle",    lcurl_multi_remove_handle    },
  {"perform",          lcurl_multi_perform          },
  {"info_read",        lcurl_multi_info_read        },
  {"setopt",           lcurl_multi_setopt           },
  {"wait",             lcurl_multi_wait             },
  {"timeout",          lcurl_multi_timeout          },

#define OPT_ENTRY(L, N, T, S) { "setopt_"#L, lcurl_multi_set_##N },
  #include "lcoptmulti.h"
  OPT_ENTRY(timerfunction, TIMERFUNCTION, TTT, 0)
#undef OPT_ENTRY

  {"close",            lcurl_multi_cleanup          },
  {"__gc",             lcurl_multi_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_multi_opt[] = {
#define OPT_ENTRY(L, N, T, S) { "OPT_MULTI_"#N, CURLMOPT_##N },
  #include "lcoptmulti.h"
  OPT_ENTRY(timerfunction, TIMERFUNCTION, TTT, 0)
#undef OPT_ENTRY

  {NULL, 0}
};

void lcurl_multi_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_MULTI, lcurl_multi_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_multi_opt);
}