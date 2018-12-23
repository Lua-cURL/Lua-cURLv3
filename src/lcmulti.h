/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCMULTI_H_
#define _LCMULTI_H_

#include "lcurl.h"
#include "lcutils.h"

typedef struct lcurl_multi_tag{
  CURLM *curl;
  lua_State *L;
  int err_mode;
  int h_ref;
  lcurl_callback_t tm;
  lcurl_callback_t sc;
}lcurl_multi_t;


#if LCURL_CC_SUPPORT_FORWARD_TYPEDEF
typedef struct lcurl_multi_tag lcurl_multi_t;
#else
struct lcurl_easy_tag;
#define lcurl_easy_t struct lcurl_easy_tag
#endif

int lcurl_multi_create(lua_State *L, int error_mode);

lcurl_multi_t *lcurl_getmulti_at(lua_State *L, int i);

#define lcurl_getmulti(L) lcurl_getmulti_at((L),1)

void lcurl_multi_initlib(lua_State *L, int nup);

void lcurl__multi_assign_lua(lua_State *L, lcurl_multi_t *p, lua_State *value, int assign_easy);

CURLMcode lcurl__multi_remove_handle(lua_State *L, lcurl_multi_t *p, lcurl_easy_t *e);

#if !LCURL_CC_SUPPORT_FORWARD_TYPEDEF
#undef lcurl_easy_t 
#endif

#endif
