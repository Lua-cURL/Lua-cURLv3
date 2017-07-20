/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#if defined(_WINDOWS) || defined(_WIN32)
#  define LCURL_WINDOWS
#endif

#ifdef LCURL_WINDOWS
#  include <winsock2.h>
#else
#  include <sys/select.h>
#endif

#include "lcurl.h"
#include "lceasy.h"
#include "lcmulti.h"
#include "lcerror.h"
#include "lcutils.h"
#include "lchttppost.h"

#define LCURL_MULTI_NAME LCURL_PREFIX" Multi"
static const char *LCURL_MULTI = LCURL_MULTI_NAME;

#if defined(DEBUG) || defined(_DEBUG)
static void lcurl__multi_validate_sate(lua_State *L, lcurl_multi_t *p){
  int top = lua_gettop(L);

  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
  assert(lua_istable(L, -1));

  lua_pushnil(L);
  while(lua_next(L, -2)){
    lcurl_easy_t *e = lcurl_geteasy_at(L, -1);
    void *ptr = lua_touserdata(L, -2);

    assert(e->curl == ptr);
    assert(e->multi == p);
    assert(e->L == p->L);

    lua_pop(L, 1);
  }

  lua_pop(L, 1);
  assert(lua_gettop(L) == top);
}
#else
#  define lcurl__multi_validate_sate(L, p) (void*)(0)
#endif

void lcurl__multi_assign_lua(lua_State *L, lcurl_multi_t *p, lua_State *value, int assign_easy){
  lcurl__multi_validate_sate(L, p);

  if((assign_easy)&&(p->L != value)){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_pushnil(L);
    while(lua_next(L, -2)){
      lcurl_easy_t *e = lcurl_geteasy_at(L, -1);
      lcurl__easy_assign_lua(L, e, value, 0);
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
  }

  p->L = value;
}

//{

int lcurl_multi_create(lua_State *L, int error_mode){
  lcurl_multi_t *p;

  lua_settop(L, 1);

  p = lutil_newudatap(L, lcurl_multi_t, LCURL_MULTI);
  p->curl = curl_multi_init();
  p->err_mode = error_mode;
  if(!p->curl) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_INTERNAL_ERROR);
  p->L = NULL;
  lcurl_util_new_weak_table(L, "v");
  p->h_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
  p->tm.cb_ref = p->tm.ud_ref = LUA_NOREF;
  p->sc.cb_ref = p->sc.ud_ref = LUA_NOREF;

  if(lua_type(L, 1) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 1, 2, 1, p->err_mode, LCURL_ERROR_MULTI, CURLM_UNKNOWN_OPTION);
    if(ret) return ret;
    assert(lua_gettop(L) == 2);
  }

  return 1;
}

lcurl_multi_t *lcurl_getmulti_at(lua_State *L, int i){
  lcurl_multi_t *p = (lcurl_multi_t *)lutil_checkudatap (L, i, LCURL_MULTI);
  luaL_argcheck (L, p != NULL, 1, LCURL_MULTI_NAME" object expected");
  return p;
}

static int lcurl_multi_to_s(lua_State *L){
  lcurl_multi_t *p = (lcurl_multi_t *)lutil_checkudatap (L, 1, LCURL_MULTI);
  lua_pushfstring(L, LCURL_MULTI_NAME" (%p)", (void*)p);
  return 1;
}

static int lcurl_multi_cleanup(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  if(p->curl){
    curl_multi_cleanup(p->curl);
    p->curl = NULL;
  }

  if(p->h_ref != LUA_NOREF){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_pushnil(L);
    while(lua_next(L, -2)){
      lcurl_easy_t *e = lcurl_geteasy_at(L, -1);
      e->multi = NULL;
      lua_pop(L, 1);
    }
    lua_pop(L, 1);
    luaL_unref(L, LCURL_LUA_REGISTRY, p->h_ref);
    p->h_ref = LUA_NOREF;
  }

  luaL_unref(L, LCURL_LUA_REGISTRY, p->tm.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->tm.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->sc.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->sc.ud_ref);
  p->tm.cb_ref = p->tm.ud_ref = LUA_NOREF;
  p->sc.cb_ref = p->sc.ud_ref = LUA_NOREF;

  lua_settop(L, 1);
  lua_pushnil(L);
  lua_rawset(L, LCURL_USERVALUES);

  return 0;
}

static int lcurl_multi_add_handle(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  lcurl_easy_t  *e = lcurl_geteasy_at(L, 2);
  CURLMcode code;
  lua_State *curL;

  if(e->multi){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, 
#if LCURL_CURL_VER_GE(7,32,1)
    CURLM_ADDED_ALREADY
#else
    CURLM_BAD_EASY_HANDLE
#endif
    );
  }

  // From doc:
  //   If you have CURLMOPT_TIMERFUNCTION set in the multi handle,
  //   that callback will be called from within this function to ask 
  //   for an updated timer so that your main event loop will get 
  //   the activity on this handle to get started.
  //
  // So we should add easy before this call
  // call chain may be like => timerfunction->socket_action->socketfunction
  lua_settop(L, 2);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
  lua_pushvalue(L, 2);
  lua_rawsetp(L, -2, e->curl);
  lua_settop(L, 1);

  // all `esay` handles have to have same L
  lcurl__easy_assign_lua(L, e, p->L, 0);

  e->multi = p;

  curL = p->L; lcurl__multi_assign_lua(L, p, L, 1);
  code = curl_multi_add_handle(p->curl, e->curl);
#ifndef LCURL_RESET_NULL_LUA
  if(curL != NULL)
#endif
  lcurl__multi_assign_lua(L, p, curL, 1);

  if(code != CURLM_OK){
    // remove
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_pushnil(L);
    lua_rawsetp(L, -2, e->curl);
    e->multi = NULL;

    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  return 1;
}

static int lcurl_multi_remove_handle(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  lcurl_easy_t  *e = lcurl_geteasy_at(L, 2);
  CURLMcode code = lcurl__multi_remove_handle(L, p, e);

  if(code != CURLM_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }

  lua_settop(L, 1);
  return 1;
}

CURLMcode lcurl__multi_remove_handle(lua_State *L, lcurl_multi_t *p, lcurl_easy_t *e){
  CURLMcode code;
  lua_State *curL;

  if(e->multi != p){
    // cURL returns CURLM_OK for such call so we do the same.
    // tested on 7.37.1
    return CURLM_OK;
  }

  curL = p->L; lcurl__multi_assign_lua(L, p, L, 1);
  code = curl_multi_remove_handle(p->curl, e->curl);
#ifndef LCURL_RESET_NULL_LUA
  if(curL != NULL)
#endif
  lcurl__multi_assign_lua(L, p, curL, 1);

  if(code == CURLM_OK){
    e->multi = NULL;
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_pushnil(L);
    lua_rawsetp(L, -2, e->curl);
    lua_pop(L, 1);
  }
 
  return code;
}

static int lcurl_multi_perform(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int running_handles = 0;
  CURLMcode code;
  lua_State *curL;

  curL = p->L; lcurl__multi_assign_lua(L, p, L, 1);
  while((code = curl_multi_perform(p->curl, &running_handles)) == CURLM_CALL_MULTI_PERFORM);
#ifndef LCURL_RESET_NULL_LUA
  if(curL != NULL)
#endif
  lcurl__multi_assign_lua(L, p, curL, 1);

  if(code != CURLM_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushnumber(L, running_handles);
  return 1;
}

static int lcurl_multi_info_read(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int msgs_in_queue = 0;
  CURLMsg *msg = curl_multi_info_read(p->curl, &msgs_in_queue);
  int remove = lua_toboolean(L, 2);

  lcurl_easy_t *e;
  if(!msg){
    lua_pushnumber(L, msgs_in_queue);
    return 1;
  }

  if(msg->msg == CURLMSG_DONE){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
    lua_rawgetp(L, -1, msg->easy_handle);
    e = lcurl_geteasy_at(L, -1);
    if(remove){
      //! @fixme We ignore any errors
      CURLMcode code;
      lua_State *curL;

      curL = p->L; lcurl__multi_assign_lua(L, p, L, 1);
      code = curl_multi_remove_handle(p->curl, e->curl);
#ifndef LCURL_RESET_NULL_LUA
      if(curL != NULL)
#endif
      lcurl__multi_assign_lua(L, p, curL, 1);

      if(CURLM_OK == code){
        e->multi = NULL;
        lua_pushnil(L);
        lua_rawsetp(L, -3, e->curl);
      }
    }
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
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
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
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
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
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
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
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushnumber(L, n);
  return 1;
}

static int lcurl_multi_socket_action(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  curl_socket_t s  = lcurl_opt_os_socket(L, 2, CURL_SOCKET_TIMEOUT);
  CURLMcode code; int n, mask;
  lua_State *curL;

  if(s == CURL_SOCKET_TIMEOUT) mask = lutil_optint64(L, 3, 0);
  else mask = lutil_checkint64(L, 3);

  curL = p->L; lcurl__multi_assign_lua(L, p, L, 1);
  code = curl_multi_socket_action(p->curl, s, mask, &n);
#ifndef LCURL_RESET_NULL_LUA
  if(curL != NULL)
#endif
  lcurl__multi_assign_lua(L, p, curL, 1);

  if(code != CURLM_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, code);
  }
  lua_pushinteger(L, n);
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
  lcurl_set_callback(L, c, 2, method);

  curl_multi_setopt(p->curl, OPT_CB, (c->cb_ref == LUA_NOREF)?0:func);
  curl_multi_setopt(p->curl, OPT_UD, (c->cb_ref == LUA_NOREF)?0:p);

  return 1;
}

//{ Timer

int lcurl_multi_timer_callback(CURLM *multi, long ms, void *arg){
  lcurl_multi_t *p = arg;
  lua_State *L = p->L;
  int n, top, ret = 0;

  assert(NULL != p->L);

  top = lua_gettop(L);
  n   = lcurl_util_push_cb(L, &p->tm);

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

//{ Socket

static int lcurl_multi_socket_callback(CURL *easy, curl_socket_t s, int what, void *arg, void *socketp){
  lcurl_multi_t *p = arg;
  lua_State *L = p->L;
  lcurl_easy_t *e;
  int n, top;

  assert(NULL != p->L);

  top = lua_gettop(L);
  n = lcurl_util_push_cb(L, &p->sc);

  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->h_ref);
  lua_rawgetp(L, -1, easy);
  e = lcurl_geteasy_at(L, -1);
  lua_remove(L, -2);
  lcurl_push_os_socket(L, s);
  lua_pushinteger(L, what);

  if(lua_pcall(L, n+2, 0, 0)){
    assert(lua_gettop(L) >= top);
    lua_settop(L, top);
    return -1; //! @todo break perform
  }

  lua_settop(L, top);
  return 0;
}

static int lcurl_multi_set_SOCKETFUNCTION(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  return lcurl_multi_set_callback(L, p, &p->sc,
    CURLMOPT_SOCKETFUNCTION, CURLMOPT_SOCKETDATA,
    "socket", lcurl_multi_socket_callback
  );
}

//}

//}

static int lcurl_multi_setopt(lua_State *L){
  lcurl_multi_t *p = lcurl_getmulti(L);
  int opt;
  
  luaL_checkany(L, 2);

  if(lua_type(L, 2) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 2, 1, 0, p->err_mode, LCURL_ERROR_MULTI, CURLM_UNKNOWN_OPTION);
    if(ret) return ret;
    lua_settop(L, 1);
    return 1;
  }

  opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S) case CURLMOPT_##N: return lcurl_multi_set_##N(L);
  switch(opt){
    #include "lcoptmulti.h"
    OPT_ENTRY(timerfunction,  TIMERFUNCTION,  TTT, 0)
    OPT_ENTRY(socketfunction, SOCKETFUNCTION, TTT, 0)
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_MULTI, CURLM_UNKNOWN_OPTION);
}

static int lcurl_multi_setdata(lua_State *L){
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  lua_insert(L, 2);
  lua_rawset(L, LCURL_USERVALUES);
  return 1;
}

static int lcurl_multi_getdata(lua_State *L){
  lua_settop(L, 1);
  lua_rawget(L, LCURL_USERVALUES);
  return 1;
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
  {"socket_action",    lcurl_multi_socket_action    },
  { "__tostring",      lcurl_multi_to_s             },

#define OPT_ENTRY(L, N, T, S) { "setopt_"#L, lcurl_multi_set_##N },
  #include "lcoptmulti.h"
  OPT_ENTRY(timerfunction,  TIMERFUNCTION,  TTT, 0)
  OPT_ENTRY(socketfunction, SOCKETFUNCTION, TTT, 0)
#undef OPT_ENTRY

  { "setdata",         lcurl_multi_setdata          },
  { "getdata",         lcurl_multi_getdata          },

  {"close",            lcurl_multi_cleanup          },
  {"__gc",             lcurl_multi_cleanup          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_multi_opt[] = {
#define OPT_ENTRY(L, N, T, S) { "OPT_MULTI_"#N, CURLMOPT_##N },
  #include "lcoptmulti.h"
  OPT_ENTRY(timerfunction,  TIMERFUNCTION,  TTT, 0)
  OPT_ENTRY(socketfunction, SOCKETFUNCTION, TTT, 0)
#undef OPT_ENTRY

  {NULL, 0}
};

void lcurl_multi_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_MULTI, lcurl_multi_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_multi_opt);
}
