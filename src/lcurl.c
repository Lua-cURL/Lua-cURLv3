#include "lcurl.h"
#include "lceasy.h"
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

static int lcurl_hpost_new_safe(lua_State *L){
  return lcurl_hpost_create(L, LCURL_ERROR_RETURN);
}

static int lcurl_easy_new(lua_State *L){
  return lcurl_easy_create(L, LCURL_ERROR_RAISE);
}

static int lcurl_hpost_new(lua_State *L){
  return lcurl_hpost_create(L, LCURL_ERROR_RAISE);
}

static int lcurl_version(lua_State *L){
  lua_pushstring(L, curl_version());
  return 1;
}

static int lcurl_version_info(lua_State *L){
  const char * const*p;
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

  lua_newtable(L);
  lua_pushstring(L, data->version);         lua_setfield(L, -2, "version");          /* LIBCURL_VERSION     */
  lua_pushnumber(L, data->version_num);     lua_setfield(L, -2, "version_num");      /* LIBCURL_VERSION_NUM */
  lua_pushstring(L, data->host);            lua_setfield(L, -2, "host");             /* OS/host/cpu/machine when configured */
  lua_pushnumber(L, data->features);        lua_setfield(L, -2, "features");         /* bitmask, see defines below */
  lua_pushstring(L, data->ssl_version);     lua_setfield(L, -2, "ssl_version");      /* human readable string */
  lua_pushnumber(L, data->ssl_version_num); lua_setfield(L, -2, "ssl_version_num");  /* not used anymore, always 0 */
  lua_pushstring(L, data->libz_version);    lua_setfield(L, -2, "libz_version");     /* human readable string */

  /* protocols is terminated by an entry with a NULL protoname */
  lua_newtable(L);
  for(p = data->protocols; *p; ++p){
    lua_pushstring(L, *p); lua_pushboolean(L, 1); lua_rawset(L, -3);
  }
  lua_setfield(L, -2, "protocols");

  /* The fields below this were added in CURLVERSION_SECOND */
  // const char *ares;
  // int ares_num;

  /* This field was added in CURLVERSION_THIRD */
  // const char *libidn;

  /* These field were added in CURLVERSION_FOURTH */

  /* Same as '_libiconv_version' if built with HAVE_ICONV */
  // int iconv_ver_num;

  // const char *libssh_version; /* human readable string */
  
  if(lua_isstring(L, 1)){
    lua_pushvalue(L, 1); lua_rawget(L, -2);
  }
  return 1;
}

static const struct luaL_Reg lcurl_functions[] = {
  {"error",           lcurl_error_new        },
  {"httppost",        lcurl_hpost_new        },
  {"easy",            lcurl_easy_new         },
  {"version",         lcurl_version          },
  {"version_info",    lcurl_version_info     },

  {NULL,NULL}
};

static const struct luaL_Reg lcurl_functions_safe[] = {
  {"error",           lcurl_error_new             },
  {"httppost",        lcurl_hpost_new_safe        },
  {"easy",            lcurl_easy_new_safe         },

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

static int luaopen_lcurl_(lua_State *L, const struct luaL_Reg *func){
  if(!LCURL_INIT){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    LCURL_INIT = 1;
  }

  lua_rawgetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);
  if(!lua_istable(L, -1)){ /* registry */
    lua_pop(L, 1);
    lua_newtable(L);
  }
  lua_newtable(L); /* library  */

  lua_pushvalue(L, -2); luaL_setfuncs(L, func, 1);
  lua_pushvalue(L, -2); lcurl_error_initlib(L, 1);
  lua_pushvalue(L, -2); lcurl_hpost_initlib(L, 1);
  lua_pushvalue(L, -2); lcurl_easy_initlib (L, 1);

  lua_pushvalue(L, -2); lua_rawsetp(L, LUA_REGISTRYINDEX, LCURL_REGISTRY);

  lua_remove(L, -2); /* registry */

  lcurl_util_set_const(L, lcurl_flags);

  return 1;
}

LCURL_EXPORT_API
int luaopen_lcurl(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions); }

LCURL_EXPORT_API
int luaopen_lcurl_safe(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions_safe); }

