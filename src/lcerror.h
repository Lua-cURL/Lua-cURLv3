/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#ifndef _LCERROR_H_
#define _LCERROR_H_

#include "lcurl.h"

#define LCURL_ERROR_CURL  1
#define LCURL_ERROR_EASY  1
#define LCURL_ERROR_MULTI 2
#define LCURL_ERROR_SHARE 3
#define LCURL_ERROR_FORM  4

#define LCURL_ERROR_RETURN 1
#define LCURL_ERROR_RAISE  2

int lcurl_fail(lua_State *L, int error_type, int code);

int lcurl_fail_ex(lua_State *L, int mode, int error_type, int code);

int lcurl_error_new(lua_State *L);

void lcurl_error_initlib(lua_State *L, int nup);

#endif
