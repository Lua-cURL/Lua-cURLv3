#ifndef _LCEASY_H_
#define _LCEASY_H_

#include "lcurl.h"

int lcurl_easy_create(lua_State *L, int error_mode);

void lcurl_easy_initlib(lua_State *L, int nup);

#endif
