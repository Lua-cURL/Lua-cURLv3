/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2021 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCEASY_H_
#define _LCEASY_H_

#include "lcurl.h"
#include "lcutils.h"
#include "lchttppost.h"

#define LCURL_LST_INDEX(N) LCURL_##N##_LIST,
#define LCURL_STR_INDEX(N)
#define LCURL_LNG_INDEX(N)
#define LCURL_OFF_INDEX(N)
#define LCURL_BLB_INDEX(N)
#define OPT_ENTRY(L, N, T, S, D) LCURL_##T##_INDEX(N)

enum {
  LCURL_LIST_DUMMY = -1,

#include"lcopteasy.h"

  LCURL_LIST_COUNT,
};

#undef LCURL_BLB_INDEX
#undef LCURL_OFF_INDEX
#undef LCURL_LST_INDEX
#undef LCURL_STR_INDEX
#undef LCURL_LNG_INDEX
#undef OPT_ENTRY

#define LCURL_EASY_MAGIC 0xEA

#if LCURL_CC_SUPPORT_FORWARD_TYPEDEF
  typedef struct lcurl_multi_tag lcurl_multi_t;
  #if LCURL_CURL_VER_GE(7,56,0)
    typedef struct lcurl_mime_tag lcurl_mime_t;
  #endif
  #if LCURL_CURL_VER_GE(7,63,0)
    typedef struct lcurl_url_tag lcurl_url_t;
  #endif
#else
  struct lcurl_multi_tag;
  #define lcurl_multi_t struct lcurl_multi_tag
  #if LCURL_CURL_VER_GE(7,56,0)
    struct lcurl_mime_tag;
    #define lcurl_mime_t struct lcurl_mime_tag
  #endif
  #if LCURL_CURL_VER_GE(7,63,0)
    struct lcurl_url_tag;
    #define lcurl_url_t struct lcurl_url_tag
  #endif
#endif

typedef struct lcurl_easy_tag{
  unsigned char magic;

  lua_State *L;
  lcurl_callback_t rd;
  lcurl_read_buffer_t rbuffer;

  lcurl_hpost_t *post;

  lcurl_multi_t *multi;

#if LCURL_CURL_VER_GE(7,56,0)
  lcurl_mime_t *mime;
#endif

  CURL *curl;
  int storage;
  int lists[LCURL_LIST_COUNT];
  int err_mode;
  lcurl_callback_t wr;
  lcurl_callback_t hd;
  lcurl_callback_t pr;
  lcurl_callback_t seek;
  lcurl_callback_t debug;
  lcurl_callback_t match;
  lcurl_callback_t chunk_bgn;
  lcurl_callback_t chunk_end;
#if LCURL_CURL_VER_GE(7,19,6)
  lcurl_callback_t ssh_key;
#endif
#if LCURL_CURL_VER_GE(7,64,0)
  lcurl_callback_t trailer;
#endif
#if LCURL_CURL_VER_GE(7,74,0) && LCURL_USE_HSTS
  lcurl_callback_t hstsread;
  lcurl_callback_t hstswrite;
#endif
}lcurl_easy_t;

int lcurl_easy_create(lua_State *L, int error_mode);

lcurl_easy_t *lcurl_geteasy_at(lua_State *L, int i);

#define lcurl_geteasy(L) lcurl_geteasy_at((L),1)

void lcurl_easy_initlib(lua_State *L, int nup);

void lcurl__easy_assign_lua(lua_State *L, lcurl_easy_t *p, lua_State *value, int assign_multi);

size_t lcurl_read_callback(lua_State *L,
  lcurl_callback_t *rd, lcurl_read_buffer_t *rbuffer,
  char *buffer, size_t size, size_t nitems
);

#if !LCURL_CC_SUPPORT_FORWARD_TYPEDEF
#undef lcurl_multi_t
#ifdef lcurl_mime_t
#undef lcurl_mime_t
#endif
#ifdef lcurl_url_t
#undef lcurl_url_t
#endif
#endif

#endif
