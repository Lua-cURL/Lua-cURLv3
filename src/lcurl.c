#include "lcurl.h"
#include "lceasy.h"
#include "lcerror.h"
#include "lchttppost.h"

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

static const struct luaL_Reg lcurl_functions[] = {
  {"error",           lcurl_error_new        },
  {"httppost",        lcurl_hpost_new        },
  {"easy",            lcurl_easy_new         },

  {NULL,NULL}
};

static const struct luaL_Reg lcurl_functions_safe[] = {
  {"error",           lcurl_error_new             },
  {"httppost",        lcurl_hpost_new_safe        },
  {"easy",            lcurl_easy_new_safe         },

  {NULL,NULL}
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

  return 1;
}

LCURL_EXPORT_API
int luaopen_lcurl(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions); }

LCURL_EXPORT_API
int luaopen_lcurl_safe(lua_State *L){ return luaopen_lcurl_(L, lcurl_functions_safe); }

