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
#include "lcmulti.h"
#include "lcshare.h"
#include "lcerror.h"
#include "lchttppost.h"
#include "lcutils.h"

/*export*/
#ifdef _WIN32
#  define LCURL_EXPORT_API __declspec(dllexport)
#else
#  define LCURL_EXPORT_API LUALIB_API
#endif

static int lcurl_easy_new_safe(lua_State *L){
  return lcurl_easy_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_multi_new_safe(lua_State *L){
  return lcurl_multi_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_share_new_safe(lua_State *L){
  return lcurl_share_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_hpost_new_safe(lua_State *L){
  return lcurl_hpost_create(L, LCURL_ERROR_RETURN);
}

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

static int lcurl_version(lua_State *L){
  lua_pushstring(L, curl_version());
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
  lua_pushnumber(L, data->version_num);     lua_setfield(L, -2, "version_num");      /* LIBCURL_VERSION_NUM */
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
  lua_setfield(L, -2, "features");         /* bitmask, see defines below */

  if(data->ssl_version){lua_pushstring(L, data->ssl_version); lua_setfield(L, -2, "ssl_version");}      /* human readable string */
  lua_pushnumber(L, data->ssl_version_num); lua_setfield(L, -2, "ssl_version_num");  /* not used anymore, always 0 */
  if(data->libz_version){lua_pushstring(L, data->libz_version); lua_setfield(L, -2, "libz_version");}   /* human readable string */

  /* protocols is terminated by an entry with a NULL protoname */
  lua_newtable(L);
  for(p = data->protocols; *p; ++p){
    push_upper(L, *p); lua_pushboolean(L, 1); lua_rawset(L, -3);
  }
  lua_setfield(L, -2, "protocols");

  if(data->age >= CURLVERSION_SECOND){
    if(data->ares){lua_pushstring(L, data->ares); lua_setfield(L, -2, "ares");}
    lua_pushnumber(L, data->ares_num);      lua_setfield(L, -2, "ares_num");
  }

  if(data->age >= CURLVERSION_THIRD){ /* added in 7.12.0 */
    if(data->libidn){lua_pushstring(L, data->libidn); lua_setfield(L, -2, "libidn");}
  }

  if(data->age >= CURLVERSION_FOURTH){ /* added in 7.16.1 */
    lua_pushnumber(L, data->iconv_ver_num); lua_setfield(L, -2, "iconv_ver_num");
    if(data->libssh_version){lua_pushstring(L, data->libssh_version);lua_setfield(L, -2, "libssh_version");}
  }

  if(lua_isstring(L, 1)){
    lua_pushvalue(L, 1); lua_rawget(L, -2);
  }

  return 1;
}

static const struct luaL_Reg lcurl_functions[] = {
  {"error",           lcurl_error_new        },
  {"form",            lcurl_hpost_new        },
  {"easy",            lcurl_easy_new         },
  {"multi",           lcurl_multi_new        },
  {"share",           lcurl_share_new        },
  {"version",         lcurl_version          },
  {"version_info",    lcurl_version_info     },

  {NULL,NULL}
};

static const struct luaL_Reg lcurl_functions_safe[] = {
  {"error",           lcurl_error_new             },
  {"form",            lcurl_hpost_new_safe        },
  {"easy",            lcurl_easy_new_safe         },
  {"multi",           lcurl_multi_new_safe        },
  {"share",           lcurl_share_new_safe        },
  {"version",         lcurl_version               },
  {"version_info",    lcurl_version_info          },

  {NULL,NULL}
};

static const lcurl_const_t lcurl_flags[] = {

#define FLG_ENTRY(N) { #N, CURL##N },
#include "lcflags.h"
#undef FLG_ENTRY

  {NULL, 0}
};

static volatile int LCURL_INIT = 0;

static const char* LCURL_REGISTRY = "LCURL Registry";
static const char* LCURL_USERVAL  = "LCURL Uservalues";

static int luaopen_lcurl_(lua_State *L, const struct luaL_Reg *func){
  if(!LCURL_INIT){
    /* Note from libcurl documentation.
     *
     * The environment it sets up is constant for the life of the program
     * and is the same for every program, so multiple calls have the same
     * effect as one call. ... This function is not thread safe.
     */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    LCURL_INIT = 1;
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

  lua_newtable(L); /* library  */

  lua_pushvalue(L, -3); lua_pushvalue(L, -3); luaL_setfuncs(L, func, 2);
  lua_pushvalue(L, -3); lua_pushvalue(L, -3); lcurl_error_initlib(L, 2);
  lua_pushvalue(L, -3); lua_pushvalue(L, -3); lcurl_hpost_initlib(L, 2);
  lua_pushvalue(L, -3); lua_pushvalue(L, -3); lcurl_easy_initlib (L, 2);
  lua_pushvalue(L, -3); lua_pushvalue(L, -3); lcurl_multi_initlib(L, 2);
  lua_pushvalue(L, -3); lua_pushvalue(L, -3); lcurl_share_initlib(L, 2);

  lua_pushvalue(L, -3); lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);
  lua_pushvalue(L, -2); lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_USERVAL);

  lua_remove(L, -2); /* registry */

  lcurl_util_set_const(L, lcurl_flags);

  return 1;
}

LCURL_EXPORT_API
int luaopen_lcurl(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions); }

LCURL_EXPORT_API
int luaopen_lcurl_safe(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions_safe); }

