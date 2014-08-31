#ifndef _LCHTTPPOST_H_
#define _LCHTTPPOST_H_

#include "lcurl.h"
#include <stdlib.h>

typedef struct lcurl_hpost_tag{
  struct curl_httppost *post;
  struct curl_httppost *last;
  int storage;
  int err_mode;
}lcurl_hpost_t;

int lcurl_hpost_create(lua_State *L, int error_mode);

void lcurl_hpost_initlib(lua_State *L, int nup);

lcurl_hpost_t *lcurl_gethpost_at(lua_State *L, int i);

#define lcurl_gethpost(L) lcurl_gethpost_at((L),1)


#endif
