/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
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

#endif
