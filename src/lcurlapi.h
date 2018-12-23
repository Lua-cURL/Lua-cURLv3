/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2018 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#ifndef _LURL_H_
#define _LURL_H_

#include "lcurl.h"
#include "lcutils.h"
#include <stdlib.h>

void lcurl_url_initlib(lua_State *L, int nup);

int lcurl_url_create(lua_State *L, int error_mode);

#endif