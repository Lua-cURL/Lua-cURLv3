#include "lcurl.h"
#include "lchttppost.h"
#include "lcerror.h"
#include "lcutils.h"

#define LCURL_HTTPPOST_NAME LCURL_PREFIX" HTTPPost"
static const char *LCURL_HTTPPOST = LCURL_HTTPPOST_NAME;

//{

int lcurl_hpost_create(lua_State *L, int error_mode){
  lcurl_hpost_t *p = lutil_newudatap(L, lcurl_hpost_t, LCURL_HTTPPOST);
  p->post = p->last = 0;
  p->storage = lcurl_storage_init(L);
  p->err_mode = error_mode;

  return 1;
}

lcurl_hpost_t *lcurl_gethpost_at(lua_State *L, int i){
  lcurl_hpost_t *p = (lcurl_hpost_t *)lutil_checkudatap (L, i, LCURL_HTTPPOST);
  luaL_argcheck (L, p != NULL, 1, LCURL_PREFIX"HTTPPost object expected");
  return p;
}

static int lcurl_hpost_add_content(lua_State *L){
  // add_buffer(name, data, [type,] [headers])
  lcurl_hpost_t *p = lcurl_gethpost(L);
  size_t name_len; const char *name = luaL_checklstring(L, 2, &name_len);
  size_t cont_len; const char *cont = luaL_checklstring(L, 3, &cont_len);
  const char *type = lua_tostring(L, 4);
  struct curl_slist *list = lcurl_util_to_slist(L, type?5:4);
  struct curl_forms forms[3];
  CURLFORMcode code;

  int i = 0;
  if(type){ forms[i].option = CURLFORM_CONTENTTYPE;    forms[i++].value = type;        }
  if(list){ forms[i].option = CURLFORM_CONTENTHEADER;  forms[i++].value = (char*)list; }
  forms[i].option = CURLFORM_END;

  code = curl_formadd(&p->post, &p->last,
    CURLFORM_PTRNAME,       name, CURLFORM_NAMELENGTH,     name_len,
    CURLFORM_PTRCONTENTS,   cont, CURLFORM_CONTENTSLENGTH, cont_len,
    CURLFORM_ARRAY,         forms,
  CURLFORM_END);

  if(code != CURL_FORMADD_OK){
    if(list) curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, code);
  }

  lcurl_storage_preserve_value(L, p->storage, 2);
  lcurl_storage_preserve_value(L, p->storage, 3);
  if(list) lcurl_storage_preserve_slist (L, p->storage, list);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_hpost_add_buffer(lua_State *L){
  // add_buffer(name, filename, data, [type,] [headers])
  lcurl_hpost_t *p = lcurl_gethpost(L);
  size_t name_len; const char *name = luaL_checklstring(L, 2, &name_len);
  const char *buff = luaL_checkstring(L, 3);
  size_t cont_len; const char *cont = luaL_checklstring(L, 4, &cont_len);
  const char *type = lua_tostring(L, 5);
  struct curl_slist *list = lcurl_util_to_slist(L, type?6:5);
  struct curl_forms forms[3];
  CURLFORMcode code;

  int i = 0;
  if(type){ forms[i].option = CURLFORM_CONTENTTYPE;    forms[i++].value = type;        }
  if(list){ forms[i].option = CURLFORM_CONTENTHEADER;  forms[i++].value = (char*)list; }
  forms[i].option = CURLFORM_END;

  code = curl_formadd(&p->post, &p->last, 
    CURLFORM_PTRNAME,   name, CURLFORM_NAMELENGTH,   name_len,
    CURLFORM_BUFFER,    buff,
    CURLFORM_BUFFERPTR, cont, CURLFORM_BUFFERLENGTH, cont_len,
    CURLFORM_ARRAY,     forms,
  CURLFORM_END);

  if(code != CURL_FORMADD_OK){
    if(list) curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, code);
  }

  lcurl_storage_preserve_value(L, p->storage, 2);
  lcurl_storage_preserve_value(L, p->storage, 4);
  if(list) lcurl_storage_preserve_slist (L, p->storage, list);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_hpost_add_file(lua_State *L){
  // add_file(name, path, [type, [fname,]] [headers])
  // add_file("Picture", "c:\\image.jpg")
  // add_file("Picture", "c:\\image.jpg", "image/jpeg")
  // add_file("Picture", "c:\\image.jpg", "image/jpeg", {"XDescript: my image"})
  // add_file("Picture", "c:\\image.jpg", "image/jpeg", "avatar.jpeg", {"XDescript: my image"})
  // add_file("Picture", "c:\\image.jpg", nil, "avatar.jpeg", {"XDescript: my image"})

  int top = lua_gettop(L);
  lcurl_hpost_t *p = lcurl_gethpost(L);
  size_t name_len; const char *name = luaL_checklstring(L, 2, &name_len);
  const char *path = luaL_checkstring(L, 3); 
  const char *type = 0, *fname = 0;
  struct curl_slist *list = NULL;
  struct curl_forms forms[3];
  CURLFORMcode code;
  int i = 0;

  if(top == 4){ /* name, path, type | headers */
    if(lua_istable(L, 4))
      list = lcurl_util_to_slist(L, 4);
    else
      type = lua_tostring(L, 4);
  }
  else if(top > 4){ /* name, path, type, fname | [fname, headers] */
    type  = lua_tostring(L, 4);
    if(top == 5){ /* name, path, type, fname | headers */
      if(lua_istable(L, 5))
        list = lcurl_util_to_slist(L, 5);
      else
        fname = lua_tostring(L, 5);
    }
    else{ /* name, path, type, fname, headers */
      fname = lua_tostring(L, 5);
      list  = lcurl_util_to_slist(L, 6);
    }
  }

  if(type){ forms[i].option = CURLFORM_CONTENTTYPE;    forms[i++].value = type;        }
  if(list){ forms[i].option = CURLFORM_CONTENTHEADER;  forms[i++].value = (char*)list; }
  forms[i].option = CURLFORM_END;

  code = curl_formadd(&p->post, &p->last, 
    CURLFORM_PTRNAME,   name, CURLFORM_NAMELENGTH,   name_len,
    CURLFORM_FILE,      path,
    CURLFORM_ARRAY,     forms,
    CURLFORM_END);

  if(code != CURL_FORMADD_OK){
    if(list) curl_slist_free_all(list);
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, code);
  }

  lcurl_storage_preserve_value(L, p->storage, 2);
  if(list) lcurl_storage_preserve_slist (L, p->storage, list);

  lua_settop(L, 1);
  return 1;
}

static int lcurl_hpost_add_files(lua_State *L){
  lcurl_hpost_t *p = lcurl_gethpost(L);
  size_t name_len; const char *name = luaL_checklstring(L, 2, &name_len);
  int i; int opt_count = 0;
  int arr_count = lua_rawlen(L, 3);
  struct curl_forms *forms;
  CURLFORMcode code;

  lua_settop(L, 3);
  if(lua_type(L, -1) != LUA_TTABLE){
    //! @fixme use library specific error codes
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, CURL_FORMADD_ILLEGAL_ARRAY);
  }

  for(i = 1; i <= arr_count; ++i){
    int n;
    lua_rawgeti(L, 3, i);

    if((lua_type(L, -1) != LUA_TTABLE) && (lua_type(L, -1) != LUA_TSTRING)){
      //! @fixme use library specific error codes
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, CURL_FORMADD_ILLEGAL_ARRAY);
    }

    n = (lua_type(L, -1) == LUA_TSTRING) ? 1: lua_rawlen(L, -1);
    if(n == 1)      opt_count += 1; // name
    else if(n == 2) opt_count += 2; // name and type
    else if(n == 3) opt_count += 3; // name, type and filename
    else{
      //! @fixme use library specific error codes
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, CURL_FORMADD_ILLEGAL_ARRAY);
    }

    lua_pop(L, 1);
  }

  if(opt_count == 0){
    lua_settop(L, 1);
    return 1;
  }

  forms = calloc(opt_count + 1, sizeof(struct curl_forms));
  if(!forms){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, CURL_FORMADD_MEMORY);
  }
  forms[opt_count].option  = CURLFORM_END;

  opt_count = 0;
  for(i = 1; i <= arr_count; ++i){
    int n;

    lua_rawgeti(L, 3, i);
    if (lua_type(L, -1) == LUA_TSTRING){
      forms[opt_count].option = CURLFORM_FILE; forms[opt_count++].value = luaL_checkstring(L, -1);
    }
    else{
      n = lua_rawlen(L, -1);
      lua_rawgeti(L, -1, 1);
      forms[opt_count].option = CURLFORM_FILE; forms[opt_count++].value = luaL_checkstring(L, -1);
      lua_pop(L, 1);
      if(n > 1){
        lua_rawgeti(L, -1, 2);
        forms[opt_count].option = CURLFORM_CONTENTTYPE; forms[opt_count++].value = luaL_checkstring(L, -1);
        lua_pop(L, 1);
      }
      if(n > 2){
        lua_rawgeti(L, -1, 3);
        forms[opt_count].option = CURLFORM_FILENAME; forms[opt_count++].value = luaL_checkstring(L, -1);
        lua_pop(L, 1);
      }
    }

    lua_pop(L, 1);
  }

  code = curl_formadd(&p->post, &p->last, 
    CURLFORM_PTRNAME, name, CURLFORM_NAMELENGTH,   name_len,
    CURLFORM_ARRAY,   forms,
    CURLFORM_END);

  free(forms);

  if(code != CURL_FORMADD_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_FORM, code);
  }

  lua_settop(L, 1);
  return 1;
}

static size_t lcurl_hpost_getter_by_buffer(void *arg, const char *buf, size_t len){
  luaL_Buffer *b = arg;
  luaL_addlstring(b, buf, len);
  return len;
}

static size_t call_writer(lua_State *L, int fn, int ctx, const char *buf, size_t len){
  int top = lua_gettop(L);
  int n = 1; // number of args
  lua_Number ret = (lua_Number)len;

  lua_pushvalue(L, fn);
  if(ctx){
    lua_pushvalue(L, ctx);
    n += 1;
  }
  lua_pushlstring(L, buf, len);

  if(lua_pcall(L, n, LUA_MULTRET, 0)) return 0;

  if(lua_gettop(L) > top){
    if(lua_isnil(L, top + 1)) return 0;
    if(lua_isboolean(L, top + 1)){
      if(!lua_toboolean(L, top + 1)) ret = 0;
    }
    else ret = lua_tonumber(L, top + 1);
  }
  lua_settop(L, top);

  return (size_t)ret;
}

static size_t lcurl_hpost_getter_by_callback1(void *arg, const char *buf, size_t len){
  lua_State *L = arg;
  assert(2 == lua_gettop(L));
  return call_writer(L, 2, 0, buf, len);
}

static size_t lcurl_hpost_getter_by_callback2(void *arg, const char *buf, size_t len){
  lua_State *L = arg;
  assert(3 == lua_gettop(L));
  return call_writer(L, 2, 3, buf, len);
}

static int lcurl_hpost_get(lua_State *L){
  // get()
  // get(fn [, ctx])
  // get(object)
  lcurl_hpost_t *p = lcurl_gethpost(L);
  CURLcode code;
  int top;

  if(lua_isnoneornil(L, 2)){
    luaL_Buffer b;
    luaL_buffinit(L, &b);

    code = curl_formget(p->post, &b, lcurl_hpost_getter_by_buffer);
    if(code != CURLE_OK){
      return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_CURL, code);
    }

    luaL_pushresult(&b);
    return 1;
  }

  if(lua_isfunction(L, 2)){
    if(lua_gettop(L) == 2){
      top  = 2;
      code = curl_formget(p->post, L, lcurl_hpost_getter_by_callback1);
    }
    else{
      top  = 3;
      lua_settop(L, 3);
      code = curl_formget(p->post, L, lcurl_hpost_getter_by_callback2);
    }
  }
  else if(lua_isuserdata(L, 2) || lua_istable(L, 2)){
    lua_settop(L, 2);
    lua_getfield(L, 2, "write");
    luaL_argcheck(L, lua_isfunction(L, -1), 2, "write method not found in object");
    assert(3 == lua_gettop(L));
    lua_insert(L, -2);
    top  = 3;
    code = curl_formget(p->post, L, lcurl_hpost_getter_by_callback2);
  }
  else{
    lua_pushliteral(L, "invalid writer type");
    return lua_error(L);
  }

  if((CURLcode)-1 == code){
    if(((lua_gettop(L) == top+1))&&(lua_isstring(L, -1))){
      return lua_error(L);
    }
    return lua_gettop(L) - top;
  }

  if(code != CURLE_OK){
    return lcurl_fail_ex(L, p->err_mode, LCURL_ERROR_CURL, code);
  }

  lua_settop(L, 1);
  return 1;
}

static int lcurl_hpost_storage(lua_State *L){
  lcurl_hpost_t *p = lcurl_gethpost(L);
  lua_rawgeti(L, LCURL_LUA_REGISTRY, p->storage);
  return 1;
}

static int lcurl_hpost_free(lua_State *L){
  lcurl_hpost_t *p = lcurl_gethpost(L);
  if(p->post){
    curl_formfree(p->post);
    p->post = p->last = 0;
  }

  if(p->storage != LUA_NOREF){
    p->storage = lcurl_storage_free(L, p->storage);
  }

  return 0;
}

//}

static const struct luaL_Reg lcurl_hpost_methods[] = {
  {"add_content",          lcurl_hpost_add_content               },
  {"add_buffer",           lcurl_hpost_add_buffer                },
  {"add_file",             lcurl_hpost_add_file                  },

  {"add_files",            lcurl_hpost_add_files                 },

  {"storage",              lcurl_hpost_storage                   },
  {"get",                  lcurl_hpost_get                       },
  {"free",                 lcurl_hpost_free                      },
  {"__gc",                 lcurl_hpost_free                      },

  {NULL,NULL}
};

void lcurl_hpost_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_HTTPPOST, lcurl_hpost_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);
}

