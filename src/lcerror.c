/*
  Author: Alexey Melnichuk <mimir@newmail.ru>

  Copyright (C) 2013-2014 Alexey Melnichuk <mimir@newmail.ru>

  Licensed according to the included 'LICENCE' document

  This file is part of lua-lcurl library.
 */

#include "lcurl.h"
#include "lcerror.h"
#include <assert.h>

#define LCURL_ERROR_NAME LCURL_PREFIX" Error"
static const char *LCURL_ERROR = LCURL_ERROR_NAME;

typedef struct lcurl_error_tag{
  int tp;
  int no;
}lcurl_error_t;

//{

static const char* lcurl_err_easy_mnemo(int err){
#define RETURN_IF(E) case CURLE_##E: return #E;

  switch (err){
    RETURN_IF ( OK                       )
    RETURN_IF ( UNSUPPORTED_PROTOCOL     )
    RETURN_IF ( FAILED_INIT              )
    RETURN_IF ( URL_MALFORMAT            )
    RETURN_IF ( NOT_BUILT_IN             )
    RETURN_IF ( COULDNT_RESOLVE_PROXY    )
    RETURN_IF ( COULDNT_RESOLVE_HOST     )
    RETURN_IF ( COULDNT_CONNECT          )
    RETURN_IF ( FTP_WEIRD_SERVER_REPLY   )
    RETURN_IF ( REMOTE_ACCESS_DENIED     )
    RETURN_IF ( FTP_ACCEPT_FAILED        )
    RETURN_IF ( FTP_WEIRD_PASS_REPLY     )
    RETURN_IF ( FTP_ACCEPT_TIMEOUT       )
    RETURN_IF ( FTP_WEIRD_PASV_REPLY     )
    RETURN_IF ( FTP_WEIRD_227_FORMAT     )
    RETURN_IF ( FTP_CANT_GET_HOST        )
    RETURN_IF ( OBSOLETE16               )
    RETURN_IF ( FTP_COULDNT_SET_TYPE     )
    RETURN_IF ( PARTIAL_FILE             )
    RETURN_IF ( FTP_COULDNT_RETR_FILE    )
    RETURN_IF ( OBSOLETE20               )
    RETURN_IF ( QUOTE_ERROR              )
    RETURN_IF ( HTTP_RETURNED_ERROR      )
    RETURN_IF ( WRITE_ERROR              )
    RETURN_IF ( OBSOLETE24               )
    RETURN_IF ( UPLOAD_FAILED            )
    RETURN_IF ( READ_ERROR               )
    RETURN_IF ( OUT_OF_MEMORY            )
    RETURN_IF ( OPERATION_TIMEDOUT       )
    RETURN_IF ( OBSOLETE29               )
    RETURN_IF ( FTP_PORT_FAILED          )
    RETURN_IF ( FTP_COULDNT_USE_REST     )
    RETURN_IF ( OBSOLETE32               )
    RETURN_IF ( RANGE_ERROR              )
    RETURN_IF ( HTTP_POST_ERROR          )
    RETURN_IF ( SSL_CONNECT_ERROR        )
    RETURN_IF ( BAD_DOWNLOAD_RESUME      )
    RETURN_IF ( FILE_COULDNT_READ_FILE   )
    RETURN_IF ( LDAP_CANNOT_BIND         )
    RETURN_IF ( LDAP_SEARCH_FAILED       )
    RETURN_IF ( OBSOLETE40               )
    RETURN_IF ( FUNCTION_NOT_FOUND       )
    RETURN_IF ( ABORTED_BY_CALLBACK      )
    RETURN_IF ( BAD_FUNCTION_ARGUMENT    )
    RETURN_IF ( OBSOLETE44               )
    RETURN_IF ( INTERFACE_FAILED         )
    RETURN_IF ( OBSOLETE46               )
    RETURN_IF ( TOO_MANY_REDIRECTS       )
    RETURN_IF ( UNKNOWN_OPTION           )
    RETURN_IF ( TELNET_OPTION_SYNTAX     )
    RETURN_IF ( OBSOLETE50               )
    RETURN_IF ( PEER_FAILED_VERIFICATION )
    RETURN_IF ( GOT_NOTHING              )
    RETURN_IF ( SSL_ENGINE_NOTFOUND      )
    RETURN_IF ( SSL_ENGINE_SETFAILED     )
    RETURN_IF ( SEND_ERROR               )
    RETURN_IF ( RECV_ERROR               )
    RETURN_IF ( OBSOLETE57               )
    RETURN_IF ( SSL_CERTPROBLEM          )
    RETURN_IF ( SSL_CIPHER               )
    RETURN_IF ( SSL_CACERT               )
    RETURN_IF ( BAD_CONTENT_ENCODING     )
    RETURN_IF ( LDAP_INVALID_URL         )
    RETURN_IF ( FILESIZE_EXCEEDED        )
    RETURN_IF ( USE_SSL_FAILED           )
    RETURN_IF ( SEND_FAIL_REWIND         )
    RETURN_IF ( SSL_ENGINE_INITFAILED    )
    RETURN_IF ( LOGIN_DENIED             )
    RETURN_IF ( TFTP_NOTFOUND            )
    RETURN_IF ( TFTP_PERM                )
    RETURN_IF ( REMOTE_DISK_FULL         )
    RETURN_IF ( TFTP_ILLEGAL             )
    RETURN_IF ( TFTP_UNKNOWNID           )
    RETURN_IF ( REMOTE_FILE_EXISTS       )
    RETURN_IF ( TFTP_NOSUCHUSER          )
    RETURN_IF ( CONV_FAILED              )
    RETURN_IF ( CONV_REQD                )
    RETURN_IF ( SSL_CACERT_BADFILE       )
    RETURN_IF ( REMOTE_FILE_NOT_FOUND    )
    RETURN_IF ( SSH                      )
    RETURN_IF ( SSL_SHUTDOWN_FAILED      )
    RETURN_IF ( AGAIN                    )
    RETURN_IF ( SSL_CRL_BADFILE          )
    RETURN_IF ( SSL_ISSUER_ERROR         )
    RETURN_IF ( FTP_PRET_FAILED          )
    RETURN_IF ( RTSP_CSEQ_ERROR          )
    RETURN_IF ( RTSP_SESSION_ERROR       )
    RETURN_IF ( FTP_BAD_FILE_LIST        )
    RETURN_IF ( CHUNK_FAILED             )
    RETURN_IF ( NO_CONNECTION_AVAILABLE  )
  }
  return "UNKNOWN";

#undef RETURN_IF
}

static const char* lcurl_err_multi_mnemo(int err){
#define RETURN_IF(E) case CURLM_##E: return #E;

  switch (err){
    RETURN_IF ( OK                 )
    RETURN_IF ( CALL_MULTI_PERFORM )
    RETURN_IF ( BAD_HANDLE         )
    RETURN_IF ( BAD_EASY_HANDLE    )
    RETURN_IF ( OUT_OF_MEMORY      )
    RETURN_IF ( INTERNAL_ERROR     )
    RETURN_IF ( BAD_SOCKET         )
    RETURN_IF ( UNKNOWN_OPTION     )
    RETURN_IF ( ADDED_ALREADY      )
  }
  return "UNKNOWN";

#undef RETURN_IF
}

static const char* lcurl_err_share_mnemo(int err){
#define RETURN_IF(E) case CURLSHE_##E: return #E;

  switch (err){
    RETURN_IF ( OK            )
    RETURN_IF ( BAD_OPTION    )
    RETURN_IF ( IN_USE        )
    RETURN_IF ( INVALID       )
    RETURN_IF ( NOMEM         )
    RETURN_IF ( NOT_BUILT_IN  )
  }
  return "UNKNOWN";

#undef RETURN_IF
}

static const char* lcurl_err_form_mnemo(int err){
#define RETURN_IF(E) case CURL_FORMADD_##E: return #E;

  switch (err){
    RETURN_IF ( OK             )
    RETURN_IF ( MEMORY         )
    RETURN_IF ( OPTION_TWICE   )
    RETURN_IF ( NULL           )
    RETURN_IF ( UNKNOWN_OPTION )
    RETURN_IF ( INCOMPLETE     )
    RETURN_IF ( ILLEGAL_ARRAY  )
    RETURN_IF ( DISABLED       )
  }
  return "UNKNOWN";

#undef RETURN_IF
}

static const char* _lcurl_err_mnemo(int tp, int err){
  switch(tp){
    case LCURL_ERROR_EASY : return lcurl_err_easy_mnemo (err);
    case LCURL_ERROR_MULTI: return lcurl_err_multi_mnemo(err);
    case LCURL_ERROR_SHARE: return lcurl_err_share_mnemo(err);
    case LCURL_ERROR_FORM : return lcurl_err_form_mnemo (err);
  }
  assert(0);
  return "<UNSUPPORTED ERROR TYPE>";
}

static const char* _lcurl_err_msg(int tp, int err){
  switch(tp){
    case LCURL_ERROR_EASY : return curl_easy_strerror (err);
    case LCURL_ERROR_MULTI: return curl_multi_strerror(err);
    case LCURL_ERROR_SHARE: return curl_share_strerror(err);
    case LCURL_ERROR_FORM : return lcurl_err_form_mnemo(err);
  }
  assert(0);
  return "<UNSUPPORTED ERROR TYPE>";
}

static void _lcurl_err_pushstring(lua_State *L, int tp, int err){
  lua_pushfstring(L, "[%s] %s (%d)",
    _lcurl_err_mnemo(tp, err),
    _lcurl_err_msg(tp, err),
    err
  );
}

//}

//{

int lcurl_error_create(lua_State *L, int error_type, int no){
  lcurl_error_t *err = lutil_newudatap(L, lcurl_error_t, LCURL_ERROR);

  assert(
    (error_type == LCURL_ERROR_EASY ) ||
    (error_type == LCURL_ERROR_MULTI) ||
    (error_type == LCURL_ERROR_SHARE) ||
    (error_type == LCURL_ERROR_FORM ) ||
    0
  );

  err->tp = error_type;
  err->no = no;
  return 1;
}

static lcurl_error_t *lcurl_geterror_at(lua_State *L, int i){
  lcurl_error_t *err = (lcurl_error_t *)lutil_checkudatap (L, i, LCURL_ERROR);
  luaL_argcheck (L, err != NULL, 1, LCURL_PREFIX"error object expected");
  return err;
}

#define lcurl_geterror(L) lcurl_geterror_at((L),1)

static int lcurl_err_no(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushinteger(L, err->no);
  return 1;
}

static int lcurl_err_msg(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushstring(L, _lcurl_err_msg(err->tp, err->no));
  return 1;
}

static int lcurl_err_mnemo(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  lua_pushstring(L, _lcurl_err_mnemo(err->tp, err->no));
  return 1;
}

static int lcurl_err_tostring(lua_State *L){
  lcurl_error_t *err = lcurl_geterror(L);
  _lcurl_err_pushstring(L, err->tp, err->no);
  return 1;
}

static int lcurl_err_equal(lua_State *L){
  lcurl_error_t *lhs = lcurl_geterror_at(L, 1);
  lcurl_error_t *rhs = lcurl_geterror_at(L, 2);
  lua_pushboolean(L, ((lhs->no == rhs->no)&&(lhs->tp == rhs->tp))?1:0);
  return 1;
}

//}

//{

int lcurl_fail_ex(lua_State *L, int mode, int error_type, int code){
  if(mode == LCURL_ERROR_RETURN){
    lua_pushnil(L);
    lcurl_error_create(L, error_type, code);
    return 2;
  }

#if LUA_VERSION_NUM >= 502 // lua 5.2
  lcurl_error_create(L, error_type, code);
#else
  _lcurl_err_pushstring(L, error_type, code);
#endif

  assert(LCURL_ERROR_RAISE == mode);

  return lua_error(L);
}

int lcurl_fail(lua_State *L, int error_type, int code){
  return lcurl_fail_ex(L, LCURL_ERROR_RETURN, error_type, code);
}

//}

int lcurl_error_new(lua_State *L){
  int tp = luaL_checkint(L, 1);
  int no = luaL_checkint(L, 2);

  //! @todo checks error type value

  lcurl_error_create(L, tp, no);
  return 1;
}

static const struct luaL_Reg lcurl_err_methods[] = {
  {"no",              lcurl_err_no               },
  {"msg",             lcurl_err_msg              },
  {"name",            lcurl_err_mnemo            },
  {"mnemo",           lcurl_err_mnemo            },
  {"__tostring",      lcurl_err_tostring         },
  {"__eq",            lcurl_err_equal            },

  {NULL,NULL}
};

void lcurl_error_initlib(lua_State *L, int nup){
  if(!lutil_createmetap(L, LCURL_ERROR, lcurl_err_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);
}
