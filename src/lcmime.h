/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2017-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCMIME_H_
#define _LCMIME_H_

#include "lcurl.h"
#include "lcutils.h"
#include <stdlib.h>

void lcurl_mime_initlib(lua_State *L, int nup);

#if LCURL_CURL_VER_GE(7,56,0)

typedef struct lcurl_mime_part_tag{
  lua_State           *L;

  lcurl_callback_t    rd;
  lcurl_read_buffer_t rbuffer;

  curl_mimepart       *part;

  struct lcurl_mime_tag *parent; /*always set and can not be changed*/

  int subpart_ref;
  int headers_ref;

  int err_mode;

  struct lcurl_mime_part_tag *next;
}lcurl_mime_part_t;

typedef struct lcurl_mime_tag{
  curl_mime            *mime;

  int storage;
  int err_mode;

  lcurl_mime_part_t   *parts;
  lcurl_mime_part_t   *parent; /*after set there no way change it*/
}lcurl_mime_t;

int lcurl_mime_create(lua_State *L, int error_mode);

lcurl_mime_t *lcurl_getmime_at(lua_State *L, int i);

#define lcurl_getmime(L) lcurl_getmime_at((L), 1)

int lcurl_mime_part_create(lua_State *L, int error_mode);

lcurl_mime_part_t *lcurl_getmimepart_at(lua_State *L, int i);

#define lcurl_getmimepart(L) lcurl_getmimepart_at((L), 1)

int lcurl_mime_set_lua(lua_State *L, lcurl_mime_t *p, lua_State *v);

#endif

#endif