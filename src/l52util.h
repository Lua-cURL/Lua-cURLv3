/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2021 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _L52UTIL_H_
#define _L52UTIL_H_

#include "lua.h"
#include "lauxlib.h"
#include <stdint.h>

#if LUA_VERSION_NUM >= 503 /* Lua 5.3 */

#ifndef luaL_checkint
#define luaL_checkint  luaL_checkinteger
#endif

#ifndef luaL_checklong
#define luaL_checklong  luaL_checkinteger
#endif

#ifndef luaL_optint
#define luaL_optint luaL_optinteger
#endif

#ifndef luaL_optlong
#define luaL_optlong luaL_optinteger
#endif

#endif

#if LUA_VERSION_NUM >= 502 /* Lua 5.2 */

/* lua_rawgetp */
/* lua_rawsetp */
/* luaL_setfuncs */
/* lua_absindex */

#ifndef lua_objlen
#define lua_objlen      lua_rawlen
#endif

int   luaL_typerror (lua_State *L, int narg, const char *tname);

#ifndef luaL_register
void luaL_register (lua_State *L, const char *libname, const luaL_Reg *l);
#endif

#ifndef lua_equal
#define lua_equal(L,idx1,idx2) lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#endif

#else                      /* Lua 5.1 */

/* functions from lua 5.2 */

# define lua_absindex(L, i) (((i)>0)?(i):((i)<=LUA_REGISTRYINDEX?(i):(lua_gettop(L)+(i)+1)))
# define lua_rawlen  lua_objlen

void  lua_rawgetp   (lua_State *L, int index, const void *p);
void  lua_rawsetp   (lua_State *L, int index, const void *p);
void  luaL_setfuncs  (lua_State *L, const luaL_Reg *l, int nup);

#endif

int   lutil_newmetatablep (lua_State *L, const void *p);
void  lutil_getmetatablep (lua_State *L, const void *p);
void  lutil_setmetatablep (lua_State *L, const void *p);

#define lutil_newudatap(L, TTYPE, TNAME) (TTYPE *)lutil_newudatap_impl(L, sizeof(TTYPE), TNAME)
int   lutil_isudatap      (lua_State *L, int ud, const void *p);
void *lutil_checkudatap   (lua_State *L, int ud, const void *p);
int   lutil_createmetap   (lua_State *L, const void *p, const luaL_Reg *methods, int nup);

void *lutil_newudatap_impl     (lua_State *L, size_t size, const void *p);

void lutil_pushuint(lua_State *L, unsigned int v);

void lutil_pushint64(lua_State *L, int64_t v);

int64_t lutil_checkint64(lua_State *L, int idx);

int64_t lutil_optint64(lua_State *L, int idx, int64_t v);

void lutil_pushnvalues(lua_State *L, int n);

int lutil_is_null(lua_State *L, int i);

void lutil_push_null(lua_State *L);

#endif
