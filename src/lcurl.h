/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCURL_H_
#define _LCURL_H_

#include "l52util.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include "curl/multi.h"

#include <assert.h>
#include <string.h>

#define LCURL_PREFIX "LcURL"

#define LCURL_LUA_REGISTRY lua_upvalueindex(1)

#define LCURL_USERVALUES lua_upvalueindex(2)

/* only for `mime` API */
#define LCURL_MIME_EASY lua_upvalueindex(3)

#endif
