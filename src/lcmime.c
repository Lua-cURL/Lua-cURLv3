/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2017-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of Lua-cURL library.
******************************************************************************/

#include "lcurl.h"
#include "lcmime.h"
#include "lceasy.h"
#include "lcerror.h"
#include "lcutils.h"

/* API Notes.
 * 1. Each mime can be root or child. If mime is a child (subpart) then curl free it
 * when parent mime is freed or when remove this part from parent. There no way reuse same mime.
 * Its not clear is it possible use mime created by one easy handle when do preform in another.
 * `m=e1:mime() e2:setopt_httpmime(m) e1:close() e2:perform()`
 * 
 * // Attach child to root (root also can have parent)
 * curl_mime_subparts(root, child);
 *
 * // curl free `child` and all its childs
 * curl_mime_subparts(root, other_child_or_null);
 *
 * // forbidden
 * curl_mime_free(child);
 */

#if LCURL_CURL_VER_GE(7,56,0)

#define LCURL_MIME_NAME LCURL_PREFIX" MIME"
static const char *LCURL_MIME = LCURL_MIME_NAME;

#define LCURL_MIME_PART_NAME LCURL_PREFIX" MIME Part"
static const char *LCURL_MIME_PART = LCURL_MIME_PART_NAME;

//{ Free mime and subparts

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

  /* remove weak reference to easy */
  lua_pushnil(L);
  lua_rawsetp(L, LCURL_MIME_EASY, p);

  return 0;
}

static void lcurl_mime_part_remove_subparts(lua_State *L, lcurl_mime_part_t *p, int free_it){
  lcurl_mime_t *sub = lcurl_mime_part_get_subparts(L, p);
  if(sub){
    assert(LUA_NOREF != p->subpart_ref);
    /* detach `subpart` mime from current mime part */
    /* if set `sub->parent = NULL` then gc for mime will try free curl_mime_free. */

    luaL_unref(L, LCURL_LUA_REGISTRY, p->subpart_ref);
    p->subpart_ref = LUA_NOREF;

    if(p->part && free_it){
      curl_mime_subparts(p->part, NULL);
    }

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

//}

int lcurl_mime_set_lua(lua_State *L, lcurl_mime_t *p, lua_State *v){
  lcurl_mime_part_t *part;
  for(part = p->parts; part; part=part->next){
    lcurl_mime_t *sub = lcurl_mime_part_get_subparts(L, part);
    if(sub) lcurl_mime_set_lua(L, sub, v);
    part->L = v;
  }
  return 0;
}

#define IS_NILORSTR(L, i) (lua_type(L, i) == LUA_TSTRING) || (lua_type(L, i) == LUA_TNIL)
#define IS_TABLE(L, i) lua_type(L, i) == LUA_TTABLE
#define IS_FALSE(L, i) ((lua_type(L, i) == LUA_TBOOLEAN) && (!lua_toboolean(L, i))) || lutil_is_null(L,i)
#define IS_OPTSTR(L, i) (IS_FALSE(L, i)) || (IS_NILORSTR(L, i))

static int lutil_isarray(lua_State *L, int i){
  int ret = 0;
  i = lua_absindex(L, i);
  lua_pushnil(L);
  if(lua_next(L, i)){
    ret = lua_isnumber(L, -2);
    lua_pop(L, 2);
  }
  return ret;
}

static int lcurl_mime_part_assign(lua_State *L, int part, const char *method){
  int top = lua_gettop(L);

  lua_pushvalue(L, part);
  lua_insert(L, -2);
  lua_getfield(L, -2, method);
  lua_insert(L, -3);
  lua_call(L, 2, LUA_MULTRET);

  return lua_gettop(L) - top + 1;
}

static const char *lcurl_mime_part_fields[] = {
  "data", "filedata", "name", "filename", "headers", "encoder", "type", NULL
};

static int lcurl_mime_part_assing_table(lua_State *L, int part, int t){
  int top = lua_gettop(L);
  const char *method; int i;

  part = lua_absindex(L, part);
  t = lua_absindex(L, t);

  if(lutil_isarray(L, t)){
    int ret;
    lua_pushvalue(L, t);
    ret = lcurl_mime_part_assign(L, part, "headers");
    if(ret != 1) return ret;

    lua_pop(L, 1);

    assert(top == lua_gettop(L));
  }
  else{
    for(i=0;method = lcurl_mime_part_fields[i]; ++i){
      lua_getfield(L, t, method);
      if(!lua_isnil(L, -1)){
        int ret = lcurl_mime_part_assign(L, part, method);
        if(ret != 1) return ret;
      }
      lua_pop(L, 1);

      assert(top == lua_gettop(L));
    }

    lua_getfield(L, t, "subparts");
    if(!lua_isnil(L, -1)){
      if(IS_FALSE(L, -1) || lcurl_getmime_at(L, -1)){
        int ret = lcurl_mime_part_assign(L, part, "subparts");
        if(ret != 1) return ret;
      }
    }
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
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

  /* weak reference from mime to easy handle */
  lua_pushvalue(L, 1);
  lua_rawsetp(L, LCURL_MIME_EASY, (void*)p);

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
  int ret;

  lua_settop(L, 2);

  ret = lcurl_mime_part_create(L, p->err_mode);
  if(ret != 1) return ret;

  /* store mime part in storage */
  lcurl_storage_preserve_value(L, p->storage, lua_absindex(L, -1));
  lcurl_mime_parts_append(p, lcurl_getmimepart_at(L, -1));

  if(lua_istable(L, 2)){
    ret = lcurl_mime_part_assing_table(L, 3, 2);
    if(ret) return ret;
  }

  return 1;
}

static int lcurl_mime_easy(lua_State *L){
  lcurl_mime_t *p = lcurl_getmime(L);
  lua_rawgetp(L, LCURL_MIME_EASY, p);
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

static int lcurl_mime_part_assing_ext(lua_State *L, int part, int i){
#define UNSET_VALUE (const char*)-1

  const char *mime_type = NULL, *mime_name = NULL, *mime_fname = NULL;
  int headers = 0;
  CURLcode ret;
  lcurl_mime_part_t *p = lcurl_getmimepart_at(L, part);

  if(IS_TABLE(L, i)) headers = i;
  else if (IS_OPTSTR(L, i)) {
    mime_type = IS_FALSE(L, i) ? UNSET_VALUE : lua_tostring(L, i);
    if(IS_TABLE(L, i+1)) headers = i+1;
    else if(IS_OPTSTR(L, i+1)){
      mime_name = IS_FALSE(L, i+1) ? UNSET_VALUE : lua_tostring(L, i+1);
      if(IS_TABLE(L, i+2)) headers = i+2;
      else if(IS_OPTSTR(L, i+2)){
        mime_fname = IS_FALSE(L, i+2) ? UNSET_VALUE : lua_tostring(L, i+2);
        if(IS_TABLE(L, i+3)) headers = i+3;
        else if(IS_FALSE(L, i+3)){
          headers = -1;
        }
      }
    }
  }

  if(mime_type){
    ret = curl_mime_type(p->part, mime_type == UNSET_VALUE ? NULL : mime_type);
    if(ret != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  if(mime_name){
    ret = curl_mime_name(p->part, mime_name == UNSET_VALUE ? NULL : mime_name);
    if(ret != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  if(mime_fname){
    ret = curl_mime_filename(p->part, mime_fname == UNSET_VALUE ? NULL : mime_fname);
    if(ret != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
    }
  }

  if(headers){
    if(-1 == headers){
      ret = curl_mime_headers(p->part, NULL, 0);
      if(ret != CURLE_OK){
        return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
      }
    }
    else
      return lcurl_mime_part_assing_table(L, part, headers);
  }

  return 0;

#undef UNSET_VALUE
}

// part:data(str[, type[, name[, filename]]][, headers])
static int lcurl_mime_part_data(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  size_t len; const char *data;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    data = NULL;
    len = 0;
  }
  else{
    data = luaL_checklstring(L, 2, &len);
    /*string too long*/
    if(len == CURL_ZERO_TERMINATED){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, CURLE_BAD_FUNCTION_ARGUMENT);
    }
  }

  /* curl_mime_data copies data */
  ret = curl_mime_data(p->part, data, len);
  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  if (lua_gettop(L) > 2){
    int res = lcurl_mime_part_assing_ext(L, 1, 3);
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
    int res = lcurl_mime_part_assing_ext(L, 1, 3);
    if (res) return res;
  }

  lua_settop(L, 1);
  return 1;
}

// part:filedata(path[, type[, name[, filename]]][, headers])
static int lcurl_mime_part_filedata(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *data = luaL_checkstring(L, 2);
  CURLcode ret;

  ret = curl_mime_filedata(p->part, data);
  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  if (lua_gettop(L) > 2){
    int res = lcurl_mime_part_assing_ext(L, 1, 3);
    if (res) return res;
  }

  lua_settop(L, 1);
  return 1;
}

// part:headers(t)
static int lcurl_mime_part_headers(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  struct curl_slist *list;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    list = NULL;
  }
  else{
    list = lcurl_util_to_slist(L, 2);
    luaL_argcheck(L, list || IS_TABLE(L, 2), 2, "array or null expected");
  }

  ret = curl_mime_headers(p->part, list, 1);

  if(ret != CURLE_OK){
    if(list) curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

// part:type(t)
static int lcurl_mime_part_type(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *mime_type;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    mime_type = NULL;
  }
  else{
    mime_type = luaL_checkstring(L, 2);
  }

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
  const char *mime_name;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    mime_name = NULL;
  }
  else{
    mime_name = luaL_checkstring(L, 2);
  }
  ret = curl_mime_name(p->part, mime_name);

  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

// part:filename(t)
static int lcurl_mime_part_filename(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *mime_name;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    mime_name = NULL;
  }
  else{
    mime_name = luaL_checkstring(L, 2);
  }
  ret = curl_mime_filename(p->part, mime_name);

  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

// part:encoder(t)
static int lcurl_mime_part_encoder(lua_State *L){
  lcurl_mime_part_t *p = lcurl_getmimepart(L);
  const char *mime_encode;
  CURLcode ret;

  if(IS_FALSE(L, 2)){
    mime_encode = NULL;
  }
  else{
    mime_encode = luaL_checkstring(L, 2);
  }
  ret = curl_mime_encoder(p->part, mime_encode);

  if(ret != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_EASY, ret);
  }

  lua_settop(L, 1);
  return 1;
}

//}

static const struct luaL_Reg lcurl_mime_methods[] = {

  {"addpart",              lcurl_mime_addpart                   },
  {"easy",                 lcurl_mime_easy                      },

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
  {"filename",             lcurl_mime_part_filename                  },
  {"type",                 lcurl_mime_part_type                      },
  {"encoder",              lcurl_mime_part_encoder                   },
  

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

