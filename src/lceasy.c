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
#include "lceasy.h"
#include "lcerror.h"
#include "lcutils.h"
#include "lchttppost.h"
#include "lcshare.h"
#include <memory.h>

static const char *LCURL_ERROR_TAG = "LCURL_ERROR_TAG";

#define LCURL_EASY_NAME LCURL_PREFIX" Easy"
static const char *LCURL_EASY = LCURL_EASY_NAME;

#if LCURL_CURL_VER_GE(7,21,5)
#  define LCURL_E_UNKNOWN_OPTION CURLE_UNKNOWN_OPTION
#else
#  define LCURL_E_UNKNOWN_OPTION CURLE_UNKNOWN_TELNET_OPTION
#endif

//{

int lcurl_easy_create(lua_State *L, int error_mode){
  lcurl_easy_t *p;
  int i;

  lua_settop(L, 1); /* options */

  p = lutil_newudatap(L, lcurl_easy_t, LCURL_EASY);

  p->curl = curl_easy_init();

  p->err_mode    = error_mode;
  if(!p->curl) return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_FAILED_INIT);
  p->L           = L;
  p->storage     = lcurl_storage_init(L);
  p->wr.cb_ref   = p->wr.ud_ref = LUA_NOREF;
  p->rd.cb_ref   = p->rd.ud_ref = LUA_NOREF;
  p->hd.cb_ref   = p->hd.ud_ref = LUA_NOREF;
  p->pr.cb_ref   = p->pr.ud_ref = LUA_NOREF;
  p->rbuffer.ref = LUA_NOREF;
  for(i = 0; i < LCURL_LIST_COUNT; ++i){
    p->lists[i] = LUA_NOREF;
  }

  if(lua_type(L, 1) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 1, 2, 1, p->err_mode, LCURL_ERROR_EASY, LCURL_E_UNKNOWN_OPTION);
    if(ret) return ret;
    assert(lua_gettop(L) == 2);
  }

  return 1;
}

lcurl_easy_t *lcurl_geteasy_at(lua_State *L, int i){
  lcurl_easy_t *p = (lcurl_easy_t *)lutil_checkudatap (L, i, LCURL_EASY);
  luaL_argcheck (L, p != NULL, 1, LCURL_EASY_NAME" expected");
  return p;
}

static int lcurl_easy_cleanup(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  int i;

  if(p->curl){
    curl_easy_cleanup(p->curl);
    p->curl = NULL;
  }

  if(p->storage != LUA_NOREF){
    p->storage = lcurl_storage_free(L, p->storage);
  }

  luaL_unref(L, LCURL_LUA_REGISTRY, p->wr.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->wr.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->rd.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->rd.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->pr.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->pr.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->hd.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->hd.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->rbuffer.ref);
  
  p->wr.cb_ref   = p->wr.ud_ref = LUA_NOREF;
  p->rd.cb_ref   = p->rd.ud_ref = LUA_NOREF;
  p->hd.cb_ref   = p->hd.ud_ref = LUA_NOREF;
  p->pr.cb_ref   = p->pr.ud_ref = LUA_NOREF;
  p->rbuffer.ref = LUA_NOREF;

  for(i = 0; i < LCURL_LIST_COUNT; ++i){
    p->lists[i] = LUA_NOREF;
  }

  lua_settop(L, 1);
  lua_pushnil(L);
  lua_rawset(L, LCURL_USERVALUES);

  return 0;
}

static int lcurl_easy_perform(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code;
  int top = 1;
  lua_settop(L, top);

  assert(p->rbuffer.ref == LUA_NOREF);

  code = curl_easy_perform(p->curl);

  if(p->rbuffer.ref != LUA_NOREF){
    luaL_unref(L, LCURL_LUA_REGISTRY, p->rbuffer.ref);
    p->rbuffer.ref = LUA_NOREF;
  }

  if(code == CURLE_OK){
    lua_settop(L, 1);
    return 1;
  }

  if((lua_gettop(L) > top)&&(lua_touserdata(L, top + 1) == LCURL_ERROR_TAG)){
    return lua_error(L);
  }

  if(code == CURLE_WRITE_ERROR){
    if(lua_gettop(L) > top){
      return lua_gettop(L) - top;
    }
  }

  if(code == CURLE_ABORTED_BY_CALLBACK){
    if(lua_gettop(L) > top){
      return lua_gettop(L) - top;
    }
  }

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
}

static int lcurl_easy_escape(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  size_t data_size; const char *data = luaL_checklstring(L, 2, &data_size);
  const char *ret = curl_easy_escape(p->curl, data, (int)data_size);
  if(!ret){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_OUT_OF_MEMORY);
  }
  lua_pushstring(L, ret);
  curl_free((char*)ret);
  return 1;
}

static int lcurl_easy_unescape(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  size_t data_size; const char *data = luaL_checklstring(L, 2, &data_size);
  int ret_size; const char *ret = curl_easy_unescape(p->curl, data, (int)data_size, &ret_size);
  if(!ret){
    lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_OUT_OF_MEMORY);
  }
  lua_pushlstring(L, ret, ret_size);
  curl_free((char*)ret);
  return 1;
}

static int lcurl_easy_reset(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  curl_easy_reset(p->curl);
  lua_settop(L, 1);

  if(p->storage != LUA_NOREF){
    lcurl_storage_free(L, p->storage);
    p->storage = lcurl_storage_init(L);
  }

  return 1;
}

//{ OPTIONS

//{ set

static int lcurl_opt_set_long_(lua_State *L, int opt){
  lcurl_easy_t *p = lcurl_geteasy(L);
  long val; CURLcode code;

  if(lua_isboolean(L, 2)) val = lua_toboolean(L, 2);
  else{
    luaL_argcheck(L, lua_type(L, 2) == LUA_TNUMBER, 2, "number or boolean expected");
    val = luaL_checklong(L, 2);
  }
  
  code = curl_easy_setopt(p->curl, opt, val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  lua_settop(L, 1);
  return 1;
}

static int lcurl_opt_set_string_(lua_State *L, int opt, int store){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code;

  luaL_argcheck(L, lua_type(L, 2) == LUA_TSTRING, 2, "string expected");

  code = curl_easy_setopt(p->curl, opt, lua_tostring(L, 2));
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  if(store)lcurl_storage_preserve_iv(L, p->storage, opt, 2);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_opt_set_slist_(lua_State *L, int opt, int list_no){
  lcurl_easy_t *p = lcurl_geteasy(L);
  struct curl_slist *list = lcurl_util_to_slist(L, 2);
  CURLcode code;
  int ref = p->lists[list_no];

  luaL_argcheck(L, list, 2, "array expected");

  if(ref != LUA_NOREF){
    struct curl_slist *tmp = lcurl_storage_remove_slist(L, p->storage, ref);
    curl_slist_free_all(tmp);
    p->lists[list_no] = LUA_NOREF;
  }

  code = curl_easy_setopt(p->curl, opt, list);

  if(code != CURLE_OK){
    curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  p->lists[list_no] = lcurl_storage_preserve_slist(L, p->storage, list);
  lua_settop(L, 1);
  return 1;
}

#define LCURL_STR_OPT(N, S) static int lcurl_easy_set_##N(lua_State *L){\
  return lcurl_opt_set_string_(L, CURLOPT_##N, (S)); \
}

#define LCURL_LST_OPT(N, S) static int lcurl_easy_set_##N(lua_State *L){\
  return lcurl_opt_set_slist_(L, CURLOPT_##N, LCURL_##N##_LIST);\
}

#define LCURL_LNG_OPT(N, S) static int lcurl_easy_set_##N(lua_State *L){\
  return lcurl_opt_set_long_(L, CURLOPT_##N);\
}

#define OPT_ENTRY(L, N, T, S, D) LCURL_##T##_OPT(N, S)

#include "lcopteasy.h"

#undef OPT_ENTRY

static int lcurl_easy_set_POSTFIELDS(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  size_t len; const char *val = luaL_checklstring(L, 2, &len);
  CURLcode code;
  if(lua_isnumber(L, 3)){
    size_t n = (size_t)lua_tonumber(L, 3);
    luaL_argcheck(L, len <= n, 3, "data length too big");
    len = n;
  }
  code = curl_easy_setopt(p->curl, CURLOPT_POSTFIELDS, val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  lcurl_storage_preserve_iv(L, p->storage, CURLOPT_POSTFIELDS, 2);
  code = curl_easy_setopt(p->curl, CURLOPT_POSTFIELDSIZE, (long)len);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  lua_settop(L, 1);
  return 1;
}

#undef LCURL_STR_OPT
#undef LCURL_LST_OPT
#undef LCURL_LNG_OPT

static size_t lcurl_hpost_read_callback(char *buffer, size_t size, size_t nitems, void *arg);

static int lcurl_easy_set_HTTPPOST(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  lcurl_hpost_t *post = lcurl_gethpost_at(L, 2);
  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_HTTPPOST, post->post);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_storage_preserve_iv(L, p->storage, CURLOPT_HTTPPOST, 2);

  if(post->stream){
    curl_easy_setopt(p->curl, CURLOPT_READFUNCTION, lcurl_hpost_read_callback);
  }

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_set_SHARE(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  lcurl_share_t *sh = lcurl_getshare_at(L, 2);
  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_SHARE, sh->curl);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_storage_preserve_iv(L, p->storage, CURLOPT_SHARE, 2);

  lua_settop(L, 1);
  return 1;
}
//}

//{ unset

static int lcurl_opt_unset_long_(lua_State *L, int opt, long val){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code;

  code = curl_easy_setopt(p->curl, opt, val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  lua_settop(L, 1);
  return 1;
}

static int lcurl_opt_unset_string_(lua_State *L, int opt, const char *val){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code;

  code = curl_easy_setopt(p->curl, opt, val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_storage_remove_i(L, p->storage, opt);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_opt_unset_slist_(lua_State *L, int opt, int list_no){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code;
  int ref = p->lists[list_no];

  code = curl_easy_setopt(p->curl, opt, NULL);

  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  if(ref != LUA_NOREF){
    struct curl_slist *list = lcurl_storage_remove_slist(L, p->storage, ref);
    curl_slist_free_all(list);
    p->lists[list_no] = LUA_NOREF;
  }

  lua_settop(L, 1);
  return 1;
}

#define LCURL_STR_OPT(N, S, D) static int lcurl_easy_unset_##N(lua_State *L){\
  return lcurl_opt_unset_string_(L, CURLOPT_##N, (D)); \
}

#define LCURL_LST_OPT(N, S, D) static int lcurl_easy_unset_##N(lua_State *L){\
  return lcurl_opt_unset_slist_(L, CURLOPT_##N, LCURL_##N##_LIST);\
}

#define LCURL_LNG_OPT(N, S, D) static int lcurl_easy_unset_##N(lua_State *L){\
  return lcurl_opt_unset_long_(L, CURLOPT_##N, (D));\
}

#define OPT_ENTRY(L, N, T, S, D) LCURL_##T##_OPT(N, S, D)

#include "lcopteasy.h"

#undef OPT_ENTRY

#undef LCURL_STR_OPT
#undef LCURL_LST_OPT
#undef LCURL_LNG_OPT

static int lcurl_easy_unset_HTTPPOST(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_HTTPPOST, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_storage_get_i(L, p->storage, CURLOPT_HTTPPOST);
  if(!lua_isnil(L, -1)){
    lcurl_hpost_t *form = lcurl_gethpost_at(L, -1);
    if(form->stream){
      /* with stream we do not set CURLOPT_READDATA but 
          we also unset it to be sure that there no way to
          call default curl reader with our READDATA
       */
      curl_easy_setopt(p->curl, CURLOPT_READFUNCTION, NULL);
      curl_easy_setopt(p->curl, CURLOPT_READDATA, NULL);
    }
    lcurl_storage_remove_i(L, p->storage, CURLOPT_HTTPPOST);
  }

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_SHARE(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);

  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_SHARE, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_storage_remove_i(L, p->storage, CURLOPT_SHARE);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_WRITEFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);

  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_WRITEFUNCTION, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  curl_easy_setopt(p->curl, CURLOPT_WRITEDATA, NULL);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_READFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);

  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_READFUNCTION, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  curl_easy_setopt(p->curl, CURLOPT_READDATA, NULL);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_HEADERFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);

  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_HEADERFUNCTION, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  curl_easy_setopt(p->curl, CURLOPT_HEADERDATA, NULL);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_PROGRESSFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);

  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_PROGRESSFUNCTION, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  curl_easy_setopt(p->curl, CURLOPT_PROGRESSDATA, NULL);

#if LCURL_CURL_VER_GE(7,32,0)
  curl_easy_setopt(p->curl, CURLOPT_XFERINFOFUNCTION, NULL);
  curl_easy_setopt(p->curl, CURLOPT_XFERINFODATA, NULL);
#endif

  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_unset_POSTFIELDS(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  CURLcode code = curl_easy_setopt(p->curl, CURLOPT_POSTFIELDS, NULL);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  curl_easy_setopt(p->curl, CURLOPT_POSTFIELDSIZE, -1);
  lcurl_storage_remove_i(L, p->storage, CURLOPT_POSTFIELDS);

  lua_settop(L, 1);
  return 1;
}

//}

//}

//{ info

static int lcurl_info_get_long_(lua_State *L, int opt){
  lcurl_easy_t *p = lcurl_geteasy(L);
  long val; CURLcode code;

  code = curl_easy_getinfo(p->curl, opt, &val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lua_pushnumber(L, val);
  return 1;
}

static int lcurl_info_get_double_(lua_State *L, int opt){
  lcurl_easy_t *p = lcurl_geteasy(L);
  double val; CURLcode code;

  code = curl_easy_getinfo(p->curl, opt, &val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lua_pushnumber(L, val);
  return 1;
}

static int lcurl_info_get_string_(lua_State *L, int opt){
  lcurl_easy_t *p = lcurl_geteasy(L);
  char *val; CURLcode code;
  
  code = curl_easy_getinfo(p->curl, opt, &val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lua_pushstring(L, val);
  return 1;
}

static int lcurl_info_get_slist_(lua_State *L, int opt){
  lcurl_easy_t *p = lcurl_geteasy(L);
  struct curl_slist *val; CURLcode code;
  
  code = curl_easy_getinfo(p->curl, opt, &val);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }

  lcurl_util_slist_to_table(L, val);
  curl_slist_free_all(val);

  return 1;
}

#define LCURL_STR_INFO(N, S) static int lcurl_easy_get_##N(lua_State *L){\
  return lcurl_info_get_string_(L, CURLINFO_##N); \
}

#define LCURL_LST_INFO(N, S) static int lcurl_easy_get_##N(lua_State *L){\
  return lcurl_info_get_slist_(L, CURLINFO_##N);\
}

#define LCURL_LNG_INFO(N, S) static int lcurl_easy_get_##N(lua_State *L){\
  return lcurl_info_get_long_(L, CURLINFO_##N);\
}

#define LCURL_DBL_INFO(N, S) static int lcurl_easy_get_##N(lua_State *L){\
  return lcurl_info_get_double_(L, CURLINFO_##N);\
}

#define OPT_ENTRY(L, N, T, S) LCURL_##T##_INFO(N, S)

#include "lcinfoeasy.h"

#undef OPT_ENTRY

#undef LCURL_STR_INFO
#undef LCURL_LST_INFO
#undef LCURL_LNG_INFO
#undef LCURL_DBL_INFO

//}

//{ CallBack

static int lcurl_easy_set_callback(lua_State *L, 
  lcurl_easy_t *p, lcurl_callback_t *c,
  int OPT_CB, int OPT_UD,
  const char *method, void *func
)
{
  CURLcode code;
  lcurl_set_callback(L, c, 2, method);

  code = curl_easy_setopt(p->curl, OPT_CB, (c->cb_ref == LUA_NOREF)?0:func);
  if((code != CURLE_OK)&&(c->cb_ref != LUA_NOREF)){
    luaL_unref(L, LCURL_LUA_REGISTRY, c->cb_ref);
    luaL_unref(L, LCURL_LUA_REGISTRY, c->ud_ref);
    c->cb_ref = c->ud_ref = LUA_NOREF;
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code); 
  }
  curl_easy_setopt(p->curl, OPT_UD, (c->cb_ref == LUA_NOREF)?0:p);

  return 1;
}

static size_t lcurl_write_callback_(lua_State*L, 
  lcurl_easy_t *p, lcurl_callback_t *c,
  char *ptr, size_t size, size_t nmemb
){
  size_t ret = size * nmemb;
  int    top = lua_gettop(L);
  int    n   = lcurl_util_push_cb(L, c);

  lua_pushlstring(L, ptr, ret);
  if(lua_pcall(L, n, LUA_MULTRET, 0)){
    assert(lua_gettop(L) >= top);
    lua_pushlightuserdata(L, (void*)LCURL_ERROR_TAG);
    lua_insert(L, top+1);
    return 0;
  }

  if(lua_gettop(L) > top){
    if(lua_isnil(L, top + 1)){
      if(lua_gettop(L) == (top+1)) lua_settop(L, top);
      return 0;
    }
    if(lua_isnumber(L, top + 1)){
      ret = (size_t)lua_tonumber(L, top + 1);
    }
    else{
      if(!lua_toboolean(L, top + 1)) ret = 0;
    }
  }

  lua_settop(L, top);
  return ret;
}

//{ Writer

static size_t lcurl_write_callback(char *ptr, size_t size, size_t nmemb, void *arg){
  lcurl_easy_t *p = arg;
  return lcurl_write_callback_(p->L, p, &p->wr, ptr, size, nmemb);
}

static int lcurl_easy_set_WRITEFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  return lcurl_easy_set_callback(L, p, &p->wr,
    CURLOPT_WRITEFUNCTION, CURLOPT_WRITEDATA,
    "write", lcurl_write_callback
  );
}

//}

//{ Reader

static size_t lcurl_read_callback(lua_State *L,
  lcurl_callback_t *rd, lcurl_read_buffer_t *rbuffer,
  char *buffer, size_t size, size_t nitems
){
  const char *data; size_t data_size;

  size_t ret = size * nitems;
  int n, top = lua_gettop(L);

  if(rbuffer->ref != LUA_NOREF){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, rbuffer->ref);
    data = luaL_checklstring(L, -1, &data_size);
    lua_pop(L, 1);

    data = data + rbuffer->off;
    data_size -= rbuffer->off;

    if(data_size > ret){
      data_size = ret;
      memcpy(buffer, data, data_size);
      rbuffer->off += data_size;
    }
    else{
      memcpy(buffer, data, data_size);
      luaL_unref(L, LCURL_LUA_REGISTRY, rbuffer->ref);
      rbuffer->ref = LUA_NOREF;
    }

    lua_settop(L, top);
    return data_size;
  }

  // buffer is clean
  assert(rbuffer->ref == LUA_NOREF);

  n = lcurl_util_push_cb(L, rd);
  lua_pushinteger(L, ret);
  if(lua_pcall(L, n, LUA_MULTRET, 0)){
    assert(lua_gettop(L) >= top);
    lua_pushlightuserdata(L, (void*)LCURL_ERROR_TAG);
    lua_insert(L, top+1);
    return CURL_READFUNC_ABORT;
  }

  if(lua_gettop(L) == top){
    return 0;
  }

  assert(lua_gettop(L) >= top);

  if(lua_type(L, top + 1) != LUA_TSTRING){
    if(lua_isnil(L, top + 1)){
      if(lua_gettop(L) == (top+1)){// only nil -> EOF
        lua_settop(L, top);
        return 0;
      }
    }
    else{
      if(lua_type(L, top + 1) == LUA_TNUMBER){
        size_t ret = lua_tonumber(L, top + 1);
        if(ret == (size_t)CURL_READFUNC_PAUSE){
          lua_settop(L, top);
          return CURL_READFUNC_PAUSE;
        }
      }
      lua_settop(L, top);
    }
    return CURL_READFUNC_ABORT;
  }

  data = lua_tolstring(L, top + 1, &data_size);
  assert(data);
  if(data_size > ret){
    data_size = ret;
    rbuffer->ref = luaL_ref(L, LCURL_LUA_REGISTRY);
    rbuffer->off = data_size;
  }
  memcpy(buffer, data, data_size);

  lua_settop(L, top);
  return data_size;
}

static size_t lcurl_easy_read_callback(char *buffer, size_t size, size_t nitems, void *arg){
  lcurl_easy_t *p = arg;
  return lcurl_read_callback(p->L, &p->rd, &p->rbuffer, buffer, size, nitems);
}

static size_t lcurl_hpost_read_callback(char *buffer, size_t size, size_t nitems, void *arg){
  lcurl_hpost_stream_t *p = arg;
  return lcurl_read_callback(p->L, &p->rd, &p->rbuffer, buffer, size, nitems);
}

static int lcurl_easy_set_READFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  return lcurl_easy_set_callback(L, p, &p->rd, 
    CURLOPT_READFUNCTION, CURLOPT_READDATA,
    "read", lcurl_easy_read_callback
  );
}

//}

//{ Header

static size_t lcurl_header_callback(char *ptr, size_t size, size_t nmemb, void *arg){
  lcurl_easy_t *p = arg;
  return lcurl_write_callback_(p->L, p, &p->hd, ptr, size, nmemb);
}

static int lcurl_easy_set_HEADERFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  return lcurl_easy_set_callback(L, p, &p->hd,
    CURLOPT_HEADERFUNCTION, CURLOPT_HEADERDATA,
    "header", lcurl_header_callback
  );
}

//}

//{ Progress

static int lcurl_xferinfo_callback(void *arg, curl_off_t dltotal, curl_off_t dlnow,
                                   curl_off_t ultotal, curl_off_t ulnow)
{
  lcurl_easy_t *p = arg;
  lua_State *L = p->L;

  int ret = 0;
  int top = lua_gettop(L);
  int n   = lcurl_util_push_cb(L, &p->pr);

  lua_pushnumber( L, (lua_Number)dltotal );
  lua_pushnumber( L, (lua_Number)dlnow   );
  lua_pushnumber( L, (lua_Number)ultotal );
  lua_pushnumber( L, (lua_Number)ulnow   );

  if(lua_pcall(L, n+3, LUA_MULTRET, 0)){
    assert(lua_gettop(L) >= top);
    lua_pushlightuserdata(L, (void*)LCURL_ERROR_TAG);
    lua_insert(L, top+1);
    return 1;
  }

  if(lua_gettop(L) > top){
    if(lua_isnil(L, top + 1)){
      if(lua_gettop(L) == (top+1)) lua_settop(L, top);
      return 1;
    }
    if(lua_isboolean(L, top + 1))
      ret = lua_toboolean(L, top + 1)?0:1;
    else{
      ret = lua_tointeger(L, top + 1);
      if(ret == 0) ret = 1; else ret = 0;
    }
  }

  lua_settop(L, top);
  return ret;
}

static int lcurl_progress_callback(void *arg, double dltotal, double dlnow,
                                   double ultotal, double ulnow)
{
  return lcurl_xferinfo_callback(arg,
    (curl_off_t)dltotal,
    (curl_off_t)dlnow,
    (curl_off_t)ultotal,
    (curl_off_t)ulnow
  );
}

static int lcurl_easy_set_PROGRESSFUNCTION(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  int n = lcurl_easy_set_callback(L, p, &p->pr,
    CURLOPT_PROGRESSFUNCTION, CURLOPT_PROGRESSDATA,
    "progress", lcurl_progress_callback
  );

#if LCURL_CURL_VER_GE(7,32,0)
  if(p->pr.cb_ref != LUA_NOREF){
    curl_easy_setopt(p->curl, CURLOPT_XFERINFOFUNCTION, lcurl_xferinfo_callback);
    curl_easy_setopt(p->curl, CURLOPT_XFERINFODATA, p);
  }
#endif

  return n;
}

//}

//}

static int lcurl_easy_setopt(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  long opt;

  luaL_checkany(L, 2);
  if(lua_type(L, 2) == LUA_TTABLE){
    int ret = lcurl_utils_apply_options(L, 2, 1, 0, p->err_mode, LCURL_ERROR_EASY, LCURL_E_UNKNOWN_OPTION);
    if(ret) return ret;
    lua_settop(L, 1);
    return 1;
  }

  opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S, D) case CURLOPT_##N: return lcurl_easy_set_##N(L);
  switch(opt){
    #include "lcopteasy.h"
    OPT_ENTRY(postfields,        POSTFIELDS,       TTT, 0, 0)
    OPT_ENTRY(httppost,          HTTPPOST,         TTT, 0, 0)
    OPT_ENTRY(share,             SHARE,            TTT, 0, 0)
    OPT_ENTRY(writefunction,     WRITEFUNCTION,    TTT, 0, 0)
    OPT_ENTRY(readfunction,      READFUNCTION,     TTT, 0, 0)
    OPT_ENTRY(headerfunction,    HEADERFUNCTION,   TTT, 0, 0)
    OPT_ENTRY(progressfunction,  PROGRESSFUNCTION, TTT, 0, 0)
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, LCURL_E_UNKNOWN_OPTION);
}

static int lcurl_easy_unsetopt(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  long opt;

  opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S, D) case CURLOPT_##N: return lcurl_easy_unset_##N(L);
  switch(opt){
    #include "lcopteasy.h"
    OPT_ENTRY(postfields,        POSTFIELDS,       TTT, 0, 0)
    OPT_ENTRY(httppost,          HTTPPOST,         TTT, 0, 0)
    OPT_ENTRY(share,             SHARE,            TTT, 0, 0)
    OPT_ENTRY(writefunction,     WRITEFUNCTION,    TTT, 0, 0)
    OPT_ENTRY(readfunction,      READFUNCTION,     TTT, 0, 0)
    OPT_ENTRY(headerfunction,    HEADERFUNCTION,   TTT, 0, 0)
    OPT_ENTRY(progressfunction,  PROGRESSFUNCTION, TTT, 0, 0)
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, LCURL_E_UNKNOWN_OPTION);
}

static int lcurl_easy_getinfo(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  long opt = luaL_checklong(L, 2);
  lua_remove(L, 2);

#define OPT_ENTRY(l, N, T, S) case CURLINFO_##N: return lcurl_easy_get_##N(L);
  switch(opt){
    #include "lcinfoeasy.h"
  }
#undef OPT_ENTRY

  return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, LCURL_E_UNKNOWN_OPTION);
}

static int lcurl_easy_pause(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  int mask = luaL_checkint(L, 2);
  CURLcode code = curl_easy_pause(p->curl, mask);
  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, code);
  }
  lua_settop(L, 1);
  return 1;
}

static int lcurl_easy_setdata(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  lua_insert(L, 2);
  lua_rawset(L, LCURL_USERVALUES);
  return 1;
}

static int lcurl_easy_getdata(lua_State *L){
  lcurl_easy_t *p = lcurl_geteasy(L);
  lua_settop(L, 1);
  lua_rawget(L, LCURL_USERVALUES);
  return 1;
}

//}

static const struct luaL_Reg lcurl_easy_methods[] = {

#define OPT_ENTRY(L, N, T, S, D) { "setopt_"#L, lcurl_easy_set_##N },
  #include "lcopteasy.h"
  OPT_ENTRY(postfields,        POSTFIELDS,       TTT, 0, 0)
  OPT_ENTRY(httppost,          HTTPPOST,         TTT, 0, 0)
  OPT_ENTRY(share,             SHARE,            TTT, 0, 0)
  OPT_ENTRY(writefunction,     WRITEFUNCTION,    TTT, 0, 0)
  OPT_ENTRY(readfunction,      READFUNCTION,     TTT, 0, 0)
  OPT_ENTRY(headerfunction,    HEADERFUNCTION,   TTT, 0, 0)
  OPT_ENTRY(progressfunction,  PROGRESSFUNCTION, TTT, 0, 0)
#undef OPT_ENTRY

#define OPT_ENTRY(L, N, T, S, D) { "unsetopt_"#L, lcurl_easy_unset_##N },
  #include "lcopteasy.h"
  OPT_ENTRY(postfields,        POSTFIELDS,       TTT, 0, 0)
  OPT_ENTRY(httppost,          HTTPPOST,         TTT, 0, 0)
  OPT_ENTRY(share,             SHARE,            TTT, 0, 0)
  OPT_ENTRY(writefunction,     WRITEFUNCTION,    TTT, 0, 0)
  OPT_ENTRY(readfunction,      READFUNCTION,     TTT, 0, 0)
  OPT_ENTRY(headerfunction,    HEADERFUNCTION,   TTT, 0, 0)
  OPT_ENTRY(progressfunction,  PROGRESSFUNCTION, TTT, 0, 0)
#undef OPT_ENTRY

#define OPT_ENTRY(L, N, T, S) { "getinfo_"#L, lcurl_easy_get_##N },
  #include "lcinfoeasy.h"
#undef OPT_ENTRY

  { "pause",    lcurl_easy_pause          },
  { "reset",    lcurl_easy_reset          },
  { "setopt",   lcurl_easy_setopt         },
  { "getinfo",  lcurl_easy_getinfo        },
  { "unsetopt", lcurl_easy_unsetopt       },
  { "escape",   lcurl_easy_escape         },
  { "unescape", lcurl_easy_unescape       },
  { "perform",  lcurl_easy_perform        },
  { "close",    lcurl_easy_cleanup        },
  { "__gc",     lcurl_easy_cleanup        },

  { "setdata",  lcurl_easy_setdata        },
  { "getdata",  lcurl_easy_getdata        },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_easy_opt[] = {

#define OPT_ENTRY(L, N, T, S, D) { "OPT_"#N, CURLOPT_##N },
#define FLG_ENTRY(N) { #N, CURL_##N },
#include "lcopteasy.h"
  OPT_ENTRY(postfields,        POSTFIELDS,       TTT, 0, 0)
  OPT_ENTRY(httppost,          HTTPPOST,         TTT, 0, 0)
  OPT_ENTRY(share,             SHARE,            TTT, 0, 0)
  OPT_ENTRY(writefunction,     WRITEFUNCTION,    TTT, 0, 0)
  OPT_ENTRY(readfunction,      READFUNCTION,     TTT, 0, 0)
  OPT_ENTRY(headerfunction,    HEADERFUNCTION,   TTT, 0, 0)
  OPT_ENTRY(progressfunction,  PROGRESSFUNCTION, TTT, 0, 0)
#undef OPT_ENTRY
#undef FLG_ENTRY

#define OPT_ENTRY(L, N, T, S) { "INFO_"#N, CURLINFO_##N },
#include "lcinfoeasy.h"
#undef OPT_ENTRY

  {NULL, 0}
};

void lcurl_easy_initlib(lua_State *L, int nup){

  /* Hack. We ensure that lcurl_easy_t and lcurl_hpost_stream_t
     compatiable for readfunction
  */
  LCURL_STATIC_ASSERT(offsetof(lcurl_easy_t, L)       == offsetof(lcurl_hpost_stream_t, L));
  LCURL_STATIC_ASSERT(offsetof(lcurl_easy_t, rd)      == offsetof(lcurl_hpost_stream_t, rd));
  LCURL_STATIC_ASSERT(offsetof(lcurl_easy_t, rbuffer) == offsetof(lcurl_hpost_stream_t, rbuffer));

  if(!lutil_createmetap(L, LCURL_EASY, lcurl_easy_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  lcurl_util_set_const(L, lcurl_easy_opt);
}