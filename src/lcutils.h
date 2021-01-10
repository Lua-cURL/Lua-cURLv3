/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2021 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#ifndef _LCUTILS_H_
#define _LCUTILS_H_

#include "lcurl.h"

#if defined(_MSC_VER) || defined(__cplusplus)
#  define LCURL_CC_SUPPORT_FORWARD_TYPEDEF 1
#elif defined(__STDC_VERSION__)
#  if __STDC_VERSION__ >= 201112
#    define LCURL_CC_SUPPORT_FORWARD_TYPEDEF 1
#  endif
#endif

#ifndef LCURL_CC_SUPPORT_FORWARD_TYPEDEF
#  define LCURL_CC_SUPPORT_FORWARD_TYPEDEF 0
#endif

#ifdef __GNUC__
  #define LCURL_UNUSED_TYPEDEF __attribute__ ((unused))
#else
  #define LCURL_UNUSED_TYPEDEF
#endif

#define LCURL_UNUSED_VAR LCURL_UNUSED_TYPEDEF

#define LCURL_MAKE_VERSION(MIN, MAJ, PAT) ((MIN<<16) + (MAJ<<8) + PAT)
#define LCURL_CURL_VER_GE(MIN, MAJ, PAT) (LIBCURL_VERSION_NUM >= LCURL_MAKE_VERSION(MIN, MAJ, PAT))

#define LCURL_CONCAT_STATIC_ASSERT_IMPL_(x, y) LCURL_CONCAT1_STATIC_ASSERT_IMPL_ (x, y)
#define LCURL_CONCAT1_STATIC_ASSERT_IMPL_(x, y) LCURL_UNUSED_TYPEDEF x##y
#define LCURL_STATIC_ASSERT(expr) typedef char LCURL_CONCAT_STATIC_ASSERT_IMPL_(static_assert_failed_at_line_, __LINE__) [(expr) ? 1 : -1]

#define LCURL_ASSERT_SAME_SIZE(a, b) LCURL_STATIC_ASSERT( sizeof(a) == sizeof(b) )
#define LCURL_ASSERT_SAME_OFFSET(a, am, b, bm) LCURL_STATIC_ASSERT( (offsetof(a,am)) == (offsetof(b,bm)) )
#define LCURL_ASSERT_SAME_FIELD_SIZE(a, am, b, bm) LCURL_ASSERT_SAME_SIZE(((a*)0)->am, ((b*)0)->bm)

typedef struct lcurl_const_tag{
  const char *name;
  long  value;
}lcurl_const_t;

typedef struct lcurl_callback_tag{
  int cb_ref;
  int ud_ref;
}lcurl_callback_t;

typedef struct lcurl_read_buffer_tag{
  int    ref;
  size_t off;
}lcurl_read_buffer_t;

int lcurl_storage_init(lua_State *L);

void lcurl_storage_preserve_value(lua_State *L, int storage, int i);

void lcurl_storage_remove_value(lua_State *L, int storage, int i);

int lcurl_storage_preserve_slist(lua_State *L, int storage, struct curl_slist * list);

struct curl_slist* lcurl_storage_remove_slist(lua_State *L, int storage, int idx);

void lcurl_storage_preserve_iv(lua_State *L, int storage, int i, int v);

void lcurl_storage_remove_i(lua_State *L, int storage, int i);

void lcurl_storage_get_i(lua_State *L, int storage, int i);

int lcurl_storage_free(lua_State *L, int storage);

struct curl_slist* lcurl_util_array_to_slist(lua_State *L, int t);

struct curl_slist* lcurl_util_to_slist(lua_State *L, int t);

void lcurl_util_slist_set(lua_State *L, int t, struct curl_slist* list);

void lcurl_util_slist_to_table(lua_State *L, struct curl_slist* list);

void lcurl_util_set_const(lua_State *L, const lcurl_const_t *reg);

int lcurl_set_callback(lua_State *L, lcurl_callback_t *c, int i, const char *method);

int lcurl_util_push_cb(lua_State *L, lcurl_callback_t *c);

int lcurl_util_new_weak_table(lua_State*L, const char *mode);

int lcurl_util_pcall_method(lua_State *L, const char *name, int nargs, int nresults, int errfunc);

int lcurl_utils_apply_options(lua_State *L, int opt, int obj, int do_close,
                              int error_mode, int error_type, int error_code
                              );

void lcurl_stack_dump (lua_State *L);

curl_socket_t lcurl_opt_os_socket(lua_State *L, int idx, curl_socket_t def);

void lcurl_push_os_socket(lua_State *L, curl_socket_t fd);

#endif
