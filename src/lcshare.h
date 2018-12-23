/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCSHARE_H_
#define _LCSHARE_H_

#include "lcurl.h"
#include "lcutils.h"

typedef struct lcurl_share_tag{
  CURLM *curl;
  int err_mode;
}lcurl_share_t;

int lcurl_share_create(lua_State *L, int error_mode);

lcurl_share_t *lcurl_getshare_at(lua_State *L, int i);

#define lcurl_getshare(L) lcurl_getshare_at((L),1)

void lcurl_share_initlib(lua_State *L, int nup);

#endif
