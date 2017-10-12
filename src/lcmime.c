/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2017 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#include "lcurl.h"
#include "lcmime.h"
#include "lceasy.h"
#include "lcerror.h"
#include "lcutils.h"

#if LCURL_CURL_VER_GE(7,56,0)

#define LCURL_MIME_NAME LCURL_PREFIX" MIME"
static const char *LCURL_MIME = LCURL_MIME_NAME;

#define LCURL_MIME_PART_NAME LCURL_PREFIX" MIME Part"
static const char *LCURL_MIME_PART = LCURL_MIME_PART_NAME;

static void lcurl_mime_part_remove_subparts(lua_State *L, lcurl_mime_part_t *p, int free_it);

static lcurl_mime_t* lcurl_mime_part_get_subparts(lua_State *L, lcurl_mime_part_t *part){
  lcurl_mime_t *sub = NULL;

  if(LUA_NOREF != part->subpart_ref){
    lua_rawgeti(L, LCURL_LUA_REGISTRY, part->subpart_ref);
    sub = lcurl_getmime_at(L, -1);
    lua_pop(L, 1);
  }

  return sub;
}

static int lcurl_mime_part_reset(lua_State *L, lcurl_mime_part_t *p){
  p->part = NULL;

  luaL_unref(L, LCURL_LUA_REGISTRY, p->rd.cb_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->rd.ud_ref);
  luaL_unref(L, LCURL_LUA_REGISTRY, p->rbuffer.ref);

  p->headers_ref = p->rbuffer.ref = p->rd.cb_ref = p->rd.ud_ref = LUA_NOREF;

  /*free only if we have no parents*/
  lcurl_mime_part_remove_subparts(L, p, 0);

  return 0;
}

static int lcurl_mime_reset(lua_State *L, lcurl_mime_t *p){
  lcurl_mime_part_t *ptr;

  /* reset all parts*/
  for(ptr = p->parts; ptr; ptr=ptr->next){
    lcurl_mime_part_reset(L, ptr);
  }

  if(LUA_NOREF != p->storage){
    p->storage = lcurl_storage_free(L, p->storage);
  }

  p->parts = p->parent = NULL;
  p->mime = NULL;

  return 0;
}

int lcurl_mime_set_lua(lua_State *L, lcurl_mime_t *p, lua_State *v){
  lcurl_mime_part_t *part;
  for(part = p->parts; part; part=part->next){
    lcurl_mime_t *sub = lcurl_mime_part_get_subparts(L, part);
    if(sub) lcurl_mime_set_lua(L, sub, v);
    part->L = v;
  }
  return 0;
}

//{ MIME

static lcurl_mime_part_t* lcurl_mime_parts_append(lcurl_mime_t *m, lcurl_mime_part_t *p){
  if(!m->parts) m->parts = p;
  else{
    lcurl_mime_part_t *ptr = m->parts;
    while(ptr->next)ptr = ptr->next;
    ptr->next = p;
  }
  return p;
}

static lcurl_mime_part_t* lcurl_mime_parts_find(lcurl_mime_t *m, lcurl_mime_part_t *p){
  lcurl_mime_part_t *ptr;

  for(ptr = m->parts; ptr; ptr = ptr->next){
    if(ptr == p) return p;
  }

  return NULL;
}

int lcurl_mime_create(lua_State *L, int error_mode){
  //! @todo make this function as method of easy handle
  lcurl_easy_t *e = lcurl_geteasy(L);

  lcurl_mime_t *p = lutil_newudatap(L, lcurl_mime_t, LCURL_MIME);

  p->mime = curl_mime_init(e->curl);

  //! @todo return more accurate error category/code
  if(!p->mime) return lcurl_fail_ex(L, error_mode, LCURL_ERROR_EASY, CURLE_FAILED_INIT);

  p->storage = lcurl_storage_init(L);
  p->err_mode = error_mode;
  p->parts = p->parent = NULL;

  return 1;
}

lcurl_mime_t *lcurl_getmime_at(lua_State *L, int i){
  lcurl_mime_t *p = (lcurl_mime_t *)lutil_checkudatap (L, i, LCURL_MIME);
  luaL_argcheck (L, p != NULL, i, LCURL_MIME_NAME" object expected");
  luaL_argcheck (L, p->mime != NULL, i, LCURL_MIME_NAME" object freed");
  return p;
}

static int lcurl_mime_to_s(lua_State *L){
  lcurl_mime_t *p = (lcurl_mime_t *)lutil_checkudatap (L, 1, LCURL_MIME);
  luaL_argcheck (L, p != NULL, 1, LCURL_MIME_NAME" object expected");

  lua_pushfstring(L, LCURL_MIME_NAME" (%p)%s", (void*)p,
    p->mime ? (p->parent ? " (subpart)" : "") : " (freed)"
  );
  return 1;
}

static int lcurl_mime_free(lua_State *L){
  lcurl_mime_t *p = (lcurl_mime_t *)lutil_checkudatap (L, 1, LCURL_MIME);
  luaL_argcheck (L, p != NULL, 1, LCURL_MIME_NAME" object expected");

  if((p->mime) && (NULL == p->parent)){
    curl_mime_free(p->mime);
  }

  return lcurl_mime_reset(L, p);
}

static int lcurl_mime_addpart(lua_State *L){
  lcurl_mime_t *p = lcurl_getmime(L);
  int ret = lcurl_mime_part_create(L, p->err_mode);
  if(ret != 1) return ret;

  /* store mime part in storage */
  lcurl_storage_preserve_value(L, p->storage, lua_absindex(L, -1));
  lcurl_mime_parts_append(p, lcurl_getmimepart_at(L, -1));
  return 1;
}

//}

//{ MIME Part

int lcurl_mime_part_create(lua_State *L, int error_mode){
  //! @todo make this function as method of mime handle
  lcurl_mime_t *m = lcurl_getmime(L);

  lcurl_mime_part_t *p = lutil_newudatap(L, lcurl_mime_part_t, LCURL_MIME_PART);

  p->part = curl_mime_addpart(m->mime);

  //! @todo return more accurate error category/code
  if(!p->part) return lcurl_fail_ex(L, error_mode, LCURL_ERROR_EASY, CURLE_FAILED_INIT);

  p->rbuffer.ref = p->rd.cb_ref = p->rd.ud_ref = LUA_NOREF;
  p->err_mode = error_mode;
  p->subpart_ref = p->headers_ref = LUA_NOREF;
  p->parent = m;

  return 1;
}

lcurl_mime_part_t *lcurl_getmimepart_at(lua_State *L, int i){
  lcurl_mime_part_t *p = (lcurl_mime_part_t *)lutil_checkudatap (L, i, LCURL_MIME_PART);
  luaL_argcheck (L, p != NULL, i, LCURL_MIME_PART_NAME" object expected");
  luaL_argcheck (L, p->part != NULL, i, LCURL_MIME_PART_NAME" object freed");
  return p;
}

static int lcurl_mime_part_to_s(lua_State *L){
  lcurl_mime_part_t *p = (lcurl_mime_part_t *)lutil_checkudatap (L, 1, LCURL_MIME_PART);
  luaL_argcheck (L, p != NULL, 1, LCURL_MIME_PART_NAME" object expected");

  lua_pushfstring(L, LCURL_MIME_PART_NAME" (%p)%s", (void*)p, p->part ? "" : " (freed)");
  return 1;
}

static int lcurl_mime_part_free(lua_State *L){
  lcurl_mime_part_t *p = (lcurl_mime_part_t *)lutil_checkudatap (L, 1, LCURL_MIME_PART);
  luaL_argcheck (L, p != NULL, 1, LCURL_MIME_PART_NAME" object expected");

  lcurl_mime_part_reset(L, p);

  return 0;
}

static void lcurl_mime_part_remove_subparts(lua_State *L, lcurl_mime_part_t *p, int free_it){
  lcurl_mime_t *sub = lcurl_mime_part_get_subparts(L, p);
  if(sub){
    assert(LUA_NOREF != p->subpart_ref);
    /* detach `subpart` mime from current mime part */

    /* if set `sub->parent = NULL` then gc for mime will try free curl_mime_free. */
    /* So do not set it unless `curl_mime_subparts(p->part, NULL)` does not free mime */

    luaL_unref(L, LCURL_LUA_REGISTRY, p->subpart_ref);
    p->subpart_ref = LUA_NOREF;

    if(p->part && free_it){
      curl_mime_subparts(p->part, NULL);
    }

    /* issues #1961 */

    /* seems curl_mime_subparts(h, NULL) free asubparts.
      so we have to invalidate all reference to all nested objects (part/mime).
      NOTE. All resources already feed. So just need set all pointers to NULL
      and free all Lua resources (like references and storages)
    */
    {
      lcurl_mime_part_t *ptr;
      /* reset all parts*/
      for(ptr = sub->parts; ptr; ptr=ptr->next){
        lcurl_mime_part_remove_subparts(L, p, 0);
      }
      lcurl_mime_reset(L, sub);
    }
  }
}

#define IS_NILORSTR(L, i) (lua_type(L, i) == LUA_TSTRING) || (lua_type(L, i) == LUA_TNIL)
#define IS_TABLE(L, i) lua_type(L, i) == LUA_TTABLE

static int lcurl_mime_part_assing_ext(lua_State *L, lcurl_mime_part_t *p, int i){
  const char *mime_type = NULL, *mime_name = NULL;
  int headers = 0;
  CURLcode ret;

  if(IS_TABLE(L, i)) headers = i;
  else if(IS_NILORSTR(L, i)){
    mime_type = lua_tostring(L, i);
    if(IS_TABLE(L, i+1)) headers = i+1;
    else if(IS_NILORSTR(L, i+1)){
      mime_name = lua_tostring(L, i+1);
      if(IS_TABLE(L, i+2)) headers = i+2;
    }
  }

  if(mime_type){
    ret = curl_mime_type(p->part, mime_type);
    if(ret != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  if(mime_name){
    ret = curl_mime_name(p->part, mime_name);
    if(ret != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  if(headers){
    struct curl_slist *list = lcurl_util_to_slist(L, headers);
    ret = curl_mime_headers(p->part, list, 1);
    if(ret != CURLE_OK){
      curl_slist_free_all(list);
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  return 0;
}

// part:data(str[, type[, name]][, headers])
static int lcurl_mime_part_data(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  size_t len; const char *data = luaL_checklstring(L, 2, &len);
  CURLcode ret;

  /*string too long*/
  if(len == CURL_ZERO_TERMINATED){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_BAD_FUNCTION_ARGUMENT);
  }

  /* curl_mime_data copies data */
  ret = curl_mime_data(p->part, data, len);
  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  if (lua_gettop(L) > 2){
    int res = lcurl_mime_part_assing_ext(L, p, 3);
    if (res) return res;
  }

  lua_settop(L, 1);
  return 1;
}

// part:subparts(mime[, type[, name]][, headers])
static int lcurl_mime_part_subparts(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  lcurl_mime_t *mime = lcurl_getmime_at(L, 2);
  CURLcode ret;

  /* we can attach mime to only one part */
  if(mime->parent){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_BAD_FUNCTION_ARGUMENT);
  }

  /* if we already have one subpart then libcurl free it so we can not use any references to it */
  lcurl_mime_part_remove_subparts(L, p, 1);

  ret = curl_mime_subparts(p->part, mime->mime);
  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_pushvalue(L, 2);
  p->subpart_ref = luaL_ref(L, LCURL_LUA_REGISTRY);
  mime->parent = p;

  if (lua_gettop(L) > 2){
    int res = lcurl_mime_part_assing_ext(L, p, 3);
    if (res) return res;
  }

  lua_settop(L, 1);
  return 1;
}

// part:filedata(path[, type[, name]][, headers])
static int lcurl_mime_part_filedata(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *data = luaL_checkstring(L, 2);
  CURLcode ret;

  ret = curl_mime_filedata(p->part, data);
  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  if (lua_gettop(L) > 2){
    int res = lcurl_mime_part_assing_ext(L, p, 3);
    if (res) return res;
  }

  lua_settop(L, 1);
  return 1;
}

// part:headers(t)
static int lcurl_mime_part_headers(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  struct curl_slist *list = lcurl_util_to_slist(L, 2);
  CURLcode ret;

  luaL_argcheck(L, list, 2, "array expected");

  ret = curl_mime_headers(p->part, list, 1);

  if(ret != CURLE_OK){
    curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

// part:type(t)
static int lcurl_mime_part_type(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *mime_type = luaL_checkstring(L, 2);
  CURLcode ret;

  ret = curl_mime_type(p->part, mime_type);

  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

// part:name(t)
static int lcurl_mime_part_name(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *mime_name = luaL_checkstring(L, 2);
  CURLcode ret;

  ret = curl_mime_name(p->part, mime_name);

  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}


//}

static const struct luaL_Reg lcurl_mime_methods[] = {

  {"addpart",              lcurl_mime_addpart                   },

  {"free",                 lcurl_mime_free                      },
  {"__gc",                 lcurl_mime_free                      },
  {"__tostring",           lcurl_mime_to_s                      },

  {NULL,NULL}
};

static const struct luaL_Reg lcurl_mime_part_methods[] = {

  {"subparts",             lcurl_mime_part_subparts                  },
  {"data",                 lcurl_mime_part_data                      },
  {"filedata",             lcurl_mime_part_filedata                  },
  {"headers",              lcurl_mime_part_headers                   },
  {"name",                 lcurl_mime_part_name                      },
  {"type",                 lcurl_mime_part_type                      },

  {"free",                 lcurl_mime_part_free                      },
  {"__gc",                 lcurl_mime_part_free                      },
  {"__tostring",           lcurl_mime_part_to_s                      },

  {NULL,NULL}
};

static int lcurl_pushvalues(lua_State *L, int nup) {
  assert(lua_gettop(L) >= nup);

  if (nup > 0) {
    int b = lua_absindex(L, -nup);
    int e = lua_absindex(L, -1);
    int i;

    lua_checkstack(L, nup);

    for(i = b; i <= e; ++i)
      lua_pushvalue(L, i);
  }

  return nup;
}

#endif

void lcurl_mime_initlib(lua_State *L, int nup){
#if LCURL_CURL_VER_GE(7,56,0)
  lcurl_pushvalues(L, nup);

  if(!lutil_createmetap(L, LCURL_MIME, lcurl_mime_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  if(!lutil_createmetap(L, LCURL_MIME_PART, lcurl_mime_part_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);
#else
  lua_pop(L, nup);
#endif
}

