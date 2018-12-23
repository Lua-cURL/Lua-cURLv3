/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LURL_H_
#define _LURL_H_

#include "lcurl.h"
#include "lcutils.h"
#include <stdlib.h>

void lcurl_url_initlib(lua_State *L, int nup);

#if LCURL_CURL_VER_GE(7,62,0)

typedef struct lcurl_url_tag {
  CURLU *url;

  int err_mode;
}lcurl_url_t;

int lcurl_url_create(lua_State *L, int error_mode);

lcurl_url_t *lcurl_geturl_at(lua_State *L, int i);

#endif

#endif