/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#ifndef _LCEASY_H_
#define _LCEASY_H_

#include "lcurl.h"
#include "lcutils.h"

#define LCURL_LST_INDEX(N) LCURL_##N##_LIST,
#define LCURL_STR_INDEX(N)
#define LCURL_LNG_INDEX(N)
#define OPT_ENTRY(L, N, T, S, D) LCURL_##T##_INDEX(N)

enum {
  LCURL_LIST_DUMMY = -1,

#include"lcopteasy.h"

  LCURL_LIST_COUNT,
};

#undef LCURL_LST_INDEX
#undef LCURL_STR_INDEX
#undef LCURL_LNG_INDEX
#undef OPT_ENTRY

typedef struct lcurl_easy_tag{
  lua_State *L;
  lcurl_callback_t rd;
  lcurl_read_buffer_t rbuffer;

  CURL *curl;
  int storage;
  int lists[LCURL_LIST_COUNT];
  int err_mode;
  lcurl_callback_t wr;
  lcurl_callback_t hd;
  lcurl_callback_t pr;
}lcurl_easy_t;

int lcurl_easy_create(lua_State *L, int error_mode);

lcurl_easy_t *lcurl_geteasy_at(lua_State *L, int i);

#define lcurl_geteasy(L) lcurl_geteasy_at((L),1)

void lcurl_easy_initlib(lua_State *L, int nup);

#endif
