/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2021 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#include "lcurl.h"
#include "lceasy.h"
#include "lcmulti.h"
#include "lcshare.h"
#include "lcerror.h"
#include "lchttppost.h"
#include "lcmime.h"
#include "lcurlapi.h"
#include "lcutils.h"

/*export*/
#ifdef _WIN32
#  define LCURL_EXPORT_API __declspec(dllexport)
#else
#  define LCURL_EXPORT_API LUALIB_API
#endif

static const char* LCURL_REGISTRY = "LCURL Registry";
static const char* LCURL_USERVAL = "LCURL Uservalues";
#if LCURL_CURL_VER_GE(7,56,0)
static const char* LCURL_MIME_EASY_MAP = "LCURL Mime easy";
#endif

#if LCURL_CURL_VER_GE(7,56,0)
#define NUP 3
#else
#define NUP 2
#endif

static volatile int LCURL_INIT = 0;

static int lcurl_init_in_mode(lua_State *L, long init_mode, int error_mode){
  if(!LCURL_INIT){
    /* Note from libcurl documentation.
     *
     * The environment it sets up is constant for the life of the program
     * and is the same for every program, so multiple calls have the same
     * effect as one call. ... This function is not thread safe.
     */
    CURLcode code = curl_global_init(init_mode);
    if (code != CURLE_OK) {
      return lcurl_fail_ex(L, error_mode, LCURL_ERROR_CURL, code);
    }
    LCURL_INIT = 1;
  }
  return 0;
}

static int lcurl_init(lua_State *L, int error_mode){
    long init_mode = CURL_GLOBAL_DEFAULT;
    if (L != NULL) {
      int type = lua_type(L, 1);
      if (type == LUA_TNUMBER) {
        init_mode = lua_tonumber(L, 1);
      }
    }
    return lcurl_init_in_mode(L, init_mode, error_mode);
}

static int lcurl_init_default(lua_State *L){
  return lcurl_init_in_mode(L, CURL_GLOBAL_DEFAULT, LCURL_ERROR_RAISE);
}

static int lcurl_init_unsafe(lua_State *L){
  return lcurl_init(L, LCURL_ERROR_RAISE);
}

static int lcurl_init_safe(lua_State *L){
  return lcurl_init(L, LCURL_ERROR_RETURN);
}

static int lcurl_easy_new_safe(lua_State *L){
  return lcurl_easy_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_multi_new_safe(lua_State *L){
  return lcurl_multi_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_share_new_safe(lua_State *L){
  return lcurl_share_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_hpost_new_safe(lua_State *L) {
  return lcurl_hpost_create(L, LCURL_ERROR_RETURN);
}

#if LCURL_CURL_VER_GE(7,62,0)

static int lcurl_url_new_safe(lua_State *L) {
  return lcurl_url_create(L, LCURL_ERROR_RETURN);
}

#endif

static int lcurl_easy_new(lua_State *L){
  return lcurl_easy_create(L, LCURL_ERROR_RAISE);
}

static int lcurl_multi_new(lua_State *L){
  return lcurl_multi_create(L, LCURL_ERROR_RAISE);
}

static int lcurl_share_new(lua_State *L){
  return lcurl_share_create(L, LCURL_ERROR_RAISE);
}

static int lcurl_hpost_new(lua_State *L){
  return lcurl_hpost_create(L, LCURL_ERROR_RAISE);
}

#if LCURL_CURL_VER_GE(7,62,0)

static int lcurl_url_new(lua_State *L) {
  return lcurl_url_create(L, LCURL_ERROR_RAISE);
}

#endif

#if LCURL_CURL_VER_GE(7,73,0)

static void lcurl_easy_option_push(lua_State *L, const struct curl_easyoption *opt) {
  lua_newtable(L);
  lua_pushliteral(L, "id");    lutil_pushuint(L, opt->id);    lua_rawset(L, -3);
  lua_pushliteral(L, "name");  lua_pushstring(L, opt->name);  lua_rawset(L, -3);
  lua_pushliteral(L, "type");  lutil_pushuint(L, opt->type);  lua_rawset(L, -3);
  lua_pushliteral(L, "flags"); lutil_pushuint(L, opt->flags); lua_rawset(L, -3);
  lua_pushliteral(L, "flags_set"); lua_newtable(L);
    lua_pushliteral(L, "alias"); lua_pushboolean(L, opt->flags & CURLOT_FLAG_ALIAS); lua_rawset(L, -3);
  lua_rawset(L, -3);
  lua_pushliteral(L, "type_name");
    switch(opt->type){
      case CURLOT_LONG    : lua_pushliteral(L, "LONG"    ); break;
      case CURLOT_VALUES  : lua_pushliteral(L, "VALUES"  ); break;
      case CURLOT_OFF_T   : lua_pushliteral(L, "OFF_T"   ); break;
      case CURLOT_OBJECT  : lua_pushliteral(L, "OBJECT"  ); break;
      case CURLOT_STRING  : lua_pushliteral(L, "STRING"  ); break;
      case CURLOT_SLIST   : lua_pushliteral(L, "SLIST"   ); break;
      case CURLOT_CBPTR   : lua_pushliteral(L, "CBPTR"   ); break;
      case CURLOT_BLOB    : lua_pushliteral(L, "BLOB"    ); break;
      case CURLOT_FUNCTION: lua_pushliteral(L, "FUNCTION"); break;
      default: lua_pushliteral(L, "UNKNOWN");
    }
  lua_rawset(L, -3);
}

static int lcurl_easy_option_next(lua_State *L) {
  const struct curl_easyoption *opt;

  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 1);

  lua_rawgeti(L, 1, 1);
  opt = lua_touserdata(L, -1);
  lua_settop(L, 1);

  opt = curl_easy_option_next(opt);
  if (!opt) {
    return 0;
  }

  lcurl_easy_option_push(L, opt);

  lua_pushlightuserdata(L, (void*)opt);
  lua_rawseti(L, 1, 1);

  return 1;
}

static int lcurl_easy_option_by_id(lua_State *L) {
  const struct curl_easyoption *opt = NULL;
  lua_Integer id = luaL_checkinteger(L, 1);

  lua_settop(L, 0);
  opt = curl_easy_option_by_id(id);
  if (!opt) {
    return 0;
  }

  lcurl_easy_option_push(L, opt);

  return 1;
}

static int lcurl_easy_option_by_name(lua_State *L) {
  const struct curl_easyoption *opt = NULL;
  const char *name = luaL_checkstring(L, 1);

  lua_settop(L, 0);
  opt = curl_easy_option_by_name(name);
  if (!opt) {
    return 0;
  }

  lcurl_easy_option_push(L, opt);

  return 1;
}

static int lcurl_easy_option_iter(lua_State *L) {
  lua_pushcfunction(L, lcurl_easy_option_next);
  lua_newtable(L);
  return 2;
}

#endif

static int lcurl_version(lua_State *L){
  lua_pushstring(L, curl_version());
  return 1;
}

static int lcurl_debug_getregistry(lua_State *L) {
  lua_rawgetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);
  return 1;
}

static int push_upper(lua_State *L, const char *str){
  char buffer[128];
  size_t i, n = strlen(str);
  char *ptr = (n < sizeof(buffer))?&buffer[0]:malloc(n + 1);
  if (!ptr) return 1;
  for(i = 0; i < n; ++i){
    if( (str[i] > 96 ) && (str[i] < 123) ) ptr[i] = str[i] - 'a' + 'A';
    else ptr[i] = str[i];
  }
  lua_pushlstring(L, ptr, n);
  if(ptr != &buffer[0]) free(ptr);
  return 0;
}

static int lcurl_version_info(lua_State *L){
  const char * const*p;
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

  lua_newtable(L);
  lua_pushstring(L, data->version);         lua_setfield(L, -2, "version");          /* LIBCURL_VERSION     */
  lutil_pushuint(L, data->version_num);     lua_setfield(L, -2, "version_num");      /* LIBCURL_VERSION_NUM */
  lua_pushstring(L, data->host);            lua_setfield(L, -2, "host");             /* OS/host/cpu/machine when configured */

  lua_newtable(L);
    lua_pushliteral(L, "IPV6");         lua_pushboolean(L, data->features & CURL_VERSION_IPV6        ); lua_rawset(L, -3);
    lua_pushliteral(L, "KERBEROS4");    lua_pushboolean(L, data->features & CURL_VERSION_KERBEROS4   ); lua_rawset(L, -3);
    lua_pushliteral(L, "SSL");          lua_pushboolean(L, data->features & CURL_VERSION_SSL         ); lua_rawset(L, -3);
    lua_pushliteral(L, "LIBZ");         lua_pushboolean(L, data->features & CURL_VERSION_LIBZ        ); lua_rawset(L, -3);
    lua_pushliteral(L, "NTLM");         lua_pushboolean(L, data->features & CURL_VERSION_NTLM        ); lua_rawset(L, -3);
    lua_pushliteral(L, "GSSNEGOTIATE"); lua_pushboolean(L, data->features & CURL_VERSION_GSSNEGOTIATE); lua_rawset(L, -3);
#if LCURL_CURL_VER_GE(7,38,0)
    lua_pushliteral(L, "GSSAPI");       lua_pushboolean(L, data->features & CURL_VERSION_GSSAPI      ); lua_rawset(L, -3);
#endif
    lua_pushliteral(L, "DEBUG");        lua_pushboolean(L, data->features & CURL_VERSION_DEBUG       ); lua_rawset(L, -3);
    lua_pushliteral(L, "ASYNCHDNS");    lua_pushboolean(L, data->features & CURL_VERSION_ASYNCHDNS   ); lua_rawset(L, -3);
    lua_pushliteral(L, "SPNEGO");       lua_pushboolean(L, data->features & CURL_VERSION_SPNEGO      ); lua_rawset(L, -3);
    lua_pushliteral(L, "LARGEFILE");    lua_pushboolean(L, data->features & CURL_VERSION_LARGEFILE   ); lua_rawset(L, -3);
    lua_pushliteral(L, "IDN");          lua_pushboolean(L, data->features & CURL_VERSION_IDN         ); lua_rawset(L, -3);
    lua_pushliteral(L, "SSPI");         lua_pushboolean(L, data->features & CURL_VERSION_SSPI        ); lua_rawset(L, -3);
    lua_pushliteral(L, "CONV");         lua_pushboolean(L, data->features & CURL_VERSION_CONV        ); lua_rawset(L, -3);
    lua_pushliteral(L, "CURLDEBUG");    lua_pushboolean(L, data->features & CURL_VERSION_CURLDEBUG   ); lua_rawset(L, -3);
#if LCURL_CURL_VER_GE(7,21,4)
    lua_pushliteral(L, "TLSAUTH_SRP");  lua_pushboolean(L, data->features & CURL_VERSION_TLSAUTH_SRP ); lua_rawset(L, -3);
#endif
#if LCURL_CURL_VER_GE(7,22,0)
    lua_pushliteral(L, "NTLM_WB");      lua_pushboolean(L, data->features & CURL_VERSION_NTLM_WB     ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_HTTP2
    lua_pushliteral(L, "HTTP2");        lua_pushboolean(L, data->features & CURL_VERSION_HTTP2       ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_HTTPS_PROXY
    lua_pushliteral(L, "HTTPS_PROXY");  lua_pushboolean(L, data->features & CURL_VERSION_HTTPS_PROXY ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_MULTI_SSL
    lua_pushliteral(L, "MULTI_SSL");    lua_pushboolean(L, data->features & CURL_VERSION_MULTI_SSL   ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_BROTLI
    lua_pushliteral(L, "BROTLI");       lua_pushboolean(L, data->features & CURL_VERSION_BROTLI      ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_ALTSVC
    lua_pushliteral(L, "ALTSVC");       lua_pushboolean(L, data->features & CURL_VERSION_ALTSVC      ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_HTTP3
    lua_pushliteral(L, "HTTP3");        lua_pushboolean(L, data->features & CURL_VERSION_HTTP3       ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_ZSTD
    lua_pushliteral(L, "ZSTD");         lua_pushboolean(L, data->features & CURL_VERSION_ZSTD        ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_UNICODE
    lua_pushliteral(L, "UNICODE");      lua_pushboolean(L, data->features & CURL_VERSION_UNICODE     ); lua_rawset(L, -3);
#endif
#ifdef CURL_VERSION_HSTS
    lua_pushliteral(L, "HSTS");         lua_pushboolean(L, data->features & CURL_VERSION_HSTS        ); lua_rawset(L, -3);
#endif

  lua_setfield(L, -2, "features");         /* bitmask, see defines below */

  if(data->ssl_version){lua_pushstring(L, data->ssl_version); lua_setfield(L, -2, "ssl_version");}      /* human readable string */
  lutil_pushuint(L, data->ssl_version_num); lua_setfield(L, -2, "ssl_version_num");  /* not used anymore, always 0 */
  if(data->libz_version){lua_pushstring(L, data->libz_version); lua_setfield(L, -2, "libz_version");}   /* human readable string */

  /* protocols is terminated by an entry with a NULL protoname */
  lua_newtable(L);
  for(p = data->protocols; *p; ++p){
    push_upper(L, *p); lua_pushboolean(L, 1); lua_rawset(L, -3);
  }
  lua_setfield(L, -2, "protocols");

  if(data->age >= CURLVERSION_SECOND){
    if(data->ares){lua_pushstring(L, data->ares); lua_setfield(L, -2, "ares");}
    lutil_pushuint(L, data->ares_num);      lua_setfield(L, -2, "ares_num");
  }

  if(data->age >= CURLVERSION_THIRD){ /* added in 7.12.0 */
    if(data->libidn){lua_pushstring(L, data->libidn); lua_setfield(L, -2, "libidn");}
  }

#if LCURL_CURL_VER_GE(7,16,1)
  if(data->age >= CURLVERSION_FOURTH){
    lutil_pushuint(L, data->iconv_ver_num); lua_setfield(L, -2, "iconv_ver_num");
    if(data->libssh_version){lua_pushstring(L, data->libssh_version);lua_setfield(L, -2, "libssh_version");}
  }
#endif

#if LCURL_CURL_VER_GE(7,57,0)
  if(data->age >= CURLVERSION_FOURTH){
    lutil_pushuint(L, data->brotli_ver_num); lua_setfield(L, -2, "brotli_ver_num");
    if(data->brotli_version){lua_pushstring(L, data->brotli_version);lua_setfield(L, -2, "brotli_version");}
  }
#endif

#if LCURL_CURL_VER_GE(7,66,0)
  if(data->age >= CURLVERSION_SIXTH){
    lutil_pushuint(L, data->nghttp2_ver_num); lua_setfield(L, -2, "nghttp2_ver_num");
    if(data->nghttp2_version){lua_pushstring(L, data->nghttp2_version);lua_setfield(L, -2, "nghttp2_version");}
    if(data->quic_version){lua_pushstring(L, data->quic_version);lua_setfield(L, -2, "quic_version");}
  }
#endif

#if LCURL_CURL_VER_GE(7,70,0)
  if(data->age >= CURLVERSION_SEVENTH){
    if(data->cainfo){lua_pushstring(L, data->cainfo);lua_setfield(L, -2, "cainfo");}
    if(data->capath){lua_pushstring(L, data->capath);lua_setfield(L, -2, "capath");}
  }
#endif

#if LCURL_CURL_VER_GE(7,72,0)
  if(data->age >= CURLVERSION_EIGHTH){
    lutil_pushuint(L, data->zstd_ver_num); lua_setfield(L, -2, "zstd_ver_num");
    if(data->zstd_version){lua_pushstring(L, data->zstd_version);lua_setfield(L, -2, "zstd_version");}
  }
#endif

  if(lua_isstring(L, 1)){
    lua_pushvalue(L, 1); lua_rawget(L, -2);
  }

  return 1;
}

static const struct luaL_Reg lcurl_functions[] = {
  {"init",            lcurl_init_unsafe      },
  {"error",           lcurl_error_new        },
  {"form",            lcurl_hpost_new        },
  {"easy",            lcurl_easy_new         },
  {"multi",           lcurl_multi_new        },
  {"share",           lcurl_share_new        },
#if LCURL_CURL_VER_GE(7,62,0)
  {"url",             lcurl_url_new          },
#endif
  {"version",         lcurl_version          },
  {"version_info",    lcurl_version_info     },
#if LCURL_CURL_VER_GE(7,73,0)
  {"ieasy_options",       lcurl_easy_option_iter    },
  {"easy_option_by_id",   lcurl_easy_option_by_id   },
  {"easy_option_by_name", lcurl_easy_option_by_name },
#endif
 
  {"__getregistry",   lcurl_debug_getregistry},

  {NULL,NULL}
};

static const struct luaL_Reg lcurl_functions_safe[] = {
  {"init",            lcurl_init_safe             },
  {"error",           lcurl_error_new             },
  {"form",            lcurl_hpost_new_safe        },
  {"easy",            lcurl_easy_new_safe         },
  {"multi",           lcurl_multi_new_safe        },
  {"share",           lcurl_share_new_safe        },
#if LCURL_CURL_VER_GE(7,62,0)
  {"url",             lcurl_url_new_safe          },
#endif
  {"version",         lcurl_version               },
  {"version_info",    lcurl_version_info          },
#if LCURL_CURL_VER_GE(7,73,0)
  {"ieasy_options",       lcurl_easy_option_iter    },
  {"easy_option_by_id",   lcurl_easy_option_by_id   },
  {"easy_option_by_name", lcurl_easy_option_by_name },
#endif

  { "__getregistry",   lcurl_debug_getregistry },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_flags[] = {

#define FLG_ENTRY(N) { #N, CURL##N },
#include "lcflags.h"
#undef FLG_ENTRY

  {NULL, 0}
};

#if LCURL_CURL_VER_GE(7,56,0)
#define LCURL_PUSH_NUP(L) lua_pushvalue(L, -NUP-1);lua_pushvalue(L, -NUP-1);lua_pushvalue(L, -NUP-1);
#else
#define LCURL_PUSH_NUP(L) lua_pushvalue(L, -NUP-1);lua_pushvalue(L, -NUP-1);
#endif

static int luaopen_lcurl_(lua_State *L, const struct luaL_Reg *func){
  if (getenv("LCURL_NO_INIT") == NULL) { // do not initialize curl if env variable LCURL_NO_INIT defined
    lcurl_init_default(L);
  }

  lua_rawgetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);
  if(!lua_istable(L, -1)){ /* registry */
    lua_pop(L, 1);
    lua_newtable(L);
  }

  lua_rawgetp(L, LUA_REGISTRYINDEX, LCURL_USERVAL);
  if(!lua_istable(L, -1)){ /* usevalues */
    lua_pop(L, 1);
    lcurl_util_new_weak_table(L, "k");
  }

#if LCURL_CURL_VER_GE(7,56,0)
  lua_rawgetp(L, LUA_REGISTRYINDEX, LCURL_MIME_EASY_MAP);
  if(!lua_istable(L, -1)){ /* Mime->Easy */
    lua_pop(L, 1);
    lcurl_util_new_weak_table(L, "v");
  }
#endif

  lua_newtable(L); /* library  */

  LCURL_PUSH_NUP(L); luaL_setfuncs(L, func, NUP);
  LCURL_PUSH_NUP(L); lcurl_error_initlib(L, NUP);
  LCURL_PUSH_NUP(L); lcurl_hpost_initlib(L, NUP);
  LCURL_PUSH_NUP(L); lcurl_easy_initlib (L, NUP);
  LCURL_PUSH_NUP(L); lcurl_mime_initlib (L, NUP);
  LCURL_PUSH_NUP(L); lcurl_multi_initlib(L, NUP);
  LCURL_PUSH_NUP(L); lcurl_share_initlib(L, NUP);
  LCURL_PUSH_NUP(L); lcurl_url_initlib  (L, NUP);

  LCURL_PUSH_NUP(L);

#if LCURL_CURL_VER_GE(7,56,0)
  lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_MIME_EASY_MAP);
#endif

  lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_USERVAL);
  lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);

  lcurl_util_set_const(L, lcurl_flags);

  lutil_push_null(L);
  lua_setfield(L, -2, "null");

  return 1;
}

LCURL_EXPORT_API
int luaopen_lcurl(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions); }

LCURL_EXPORT_API
int luaopen_lcurl_safe(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions_safe); }

