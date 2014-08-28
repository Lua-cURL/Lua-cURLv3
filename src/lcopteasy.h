#undef INTERFACE
#undef BUFFERSIZE
#undef TCP_NODELAY
#undef TCP_KEEPALIVE

/* Before version 7.17.0, strings were not copied.
   Instead the user was forced keep them available 
   until libcurl no longer needed them.
*/

#ifndef LCURL_STORE_STRING
#  if LCURL_CURL_VER_GE(7,17,0)
#    define LCURL_STORE_STRING 0
#  else
#    define LCURL_STORE_STRING 1
#  endif
#endif

#ifndef OPT_ENTRY
#  define OPT_ENTRY(a,b,c,d)
#  define OPT_ENTRY_IS_NULL
#endif

#ifndef FLG_ENTRY
#  define FLG_ENTRY(a)
#  define FLG_ENTRY_IS_NULL
#endif

OPT_ENTRY( verbose,                VERBOSE,                  LNG, 0 )
OPT_ENTRY( header,                 HEADER,                   LNG, 0 )
OPT_ENTRY( noprogress,             NOPROGRESS,               LNG, 0 )
OPT_ENTRY( nosignal,               NOSIGNAL,                 LNG, 0 )
OPT_ENTRY( wildcardmatch,          WILDCARDMATCH,            LNG, 0 )

OPT_ENTRY( url,                    URL,                      STR, LCURL_STORE_STRING )
OPT_ENTRY( failonerror,            FAILONERROR,              LNG, 0 )

OPT_ENTRY( protocols,              PROTOCOLS,                LNG, 0 )
OPT_ENTRY( redir_protocols,        REDIR_PROTOCOLS,          LNG, 0 )
OPT_ENTRY( proxy,                  PROXY,                    STR, LCURL_STORE_STRING )
OPT_ENTRY( proxyport,              PROXYPORT,                LNG, 0 )
OPT_ENTRY( proxytype,              PROXYTYPE,                LNG, 0 )
OPT_ENTRY( noproxy,                NOPROXY,                  STR, LCURL_STORE_STRING )
OPT_ENTRY( httpproxytunnel,        HTTPPROXYTUNNEL,          LNG, 0 )
OPT_ENTRY( socks5_gssapi_service,  SOCKS5_GSSAPI_SERVICE,    STR, LCURL_STORE_STRING )
OPT_ENTRY( socks5_gssapi_nec,      SOCKS5_GSSAPI_NEC,        LNG, 0 )
OPT_ENTRY( interface,              INTERFACE,                STR, LCURL_STORE_STRING )
OPT_ENTRY( localport,              LOCALPORT,                LNG, 0 )
OPT_ENTRY( localportrange,         LOCALPORTRANGE,           LNG, 0 )
OPT_ENTRY( dns_cache_timeout,      DNS_CACHE_TIMEOUT,        LNG, 0 )
OPT_ENTRY( dns_use_global_cache,   DNS_USE_GLOBAL_CACHE,     LNG, 0 )
OPT_ENTRY( buffersize,             BUFFERSIZE,               LNG, 0 )
OPT_ENTRY( port,                   PORT,                     LNG, 0 )
OPT_ENTRY( tcp_nodelay,            TCP_NODELAY,              LNG, 0 )
OPT_ENTRY( address_scope,          ADDRESS_SCOPE,            LNG, 0 )
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( tcp_keepalive,          TCP_KEEPALIVE,            LNG, 0 )
OPT_ENTRY( tcp_keepidle,           TCP_KEEPIDLE,             LNG, 0 )
OPT_ENTRY( tcp_keepintvl,          TCP_KEEPINTVL,            LNG, 0 )
#endif

OPT_ENTRY( netrc,                  NETRC,                    LNG, 0 )
OPT_ENTRY( netrc_file,             NETRC_FILE,               STR, LCURL_STORE_STRING )
OPT_ENTRY( userpwd,                USERPWD,                  STR, LCURL_STORE_STRING )
OPT_ENTRY( proxyuserpwd,           PROXYUSERPWD,             STR, LCURL_STORE_STRING )
OPT_ENTRY( username,               USERNAME,                 STR, LCURL_STORE_STRING )
OPT_ENTRY( password,               PASSWORD,                 STR, LCURL_STORE_STRING )
#if LCURL_CURL_VER_GE(7,31,0)
OPT_ENTRY( login_options,          LOGIN_OPTIONS,            STR, LCURL_STORE_STRING )
#endif
OPT_ENTRY( proxyusername,          PROXYUSERNAME,            STR, LCURL_STORE_STRING )
OPT_ENTRY( proxypassword,          PROXYPASSWORD,            STR, LCURL_STORE_STRING )
OPT_ENTRY( httpauth,               HTTPAUTH,                 STR, LCURL_STORE_STRING )
OPT_ENTRY( tlsauth_username,       TLSAUTH_USERNAME,         STR, LCURL_STORE_STRING )
OPT_ENTRY( tlsauth_password,       TLSAUTH_PASSWORD,         STR, LCURL_STORE_STRING )
OPT_ENTRY( proxyauth,              PROXYAUTH,                LNG, 0 )
#if LCURL_CURL_VER_GE(7,31,0)
OPT_ENTRY( sasl_ir,                SASL_IR,                  LNG, 0 )
#endif
#if LCURL_CURL_VER_GE(7,33,0)
OPT_ENTRY( xoauth2_bearer,         XOAUTH2_BEARER,           STR, LCURL_STORE_STRING )
#endif

OPT_ENTRY( autoreferer,            AUTOREFERER,              LNG, 0 )
OPT_ENTRY( accept_encoding,        ACCEPT_ENCODING,          STR, LCURL_STORE_STRING )
OPT_ENTRY( transfer_encoding,      TRANSFER_ENCODING,        LNG, 0 )
OPT_ENTRY( followlocation,         FOLLOWLOCATION,           LNG, 0 )
OPT_ENTRY( unrestricted_auth,      UNRESTRICTED_AUTH,        LNG, 0 )
OPT_ENTRY( maxredirs,              MAXREDIRS,                LNG, 0 )
OPT_ENTRY( postredir,              POSTREDIR,                LNG, 0 )
OPT_ENTRY( put,                    PUT,                      LNG, 0 )
OPT_ENTRY( post,                   POST,                     LNG, 0 )
OPT_ENTRY( referer,                REFERER,                  STR, LCURL_STORE_STRING )
OPT_ENTRY( useragent,              USERAGENT,                STR, LCURL_STORE_STRING )
#if LCURL_CURL_VER_GE(7,37,0)
OPT_ENTRY( headeropt,              HEADEROPT,                LNG, 0 )
#endif
OPT_ENTRY( httpheader,             HTTPHEADER,               LST, 0 )
#if LCURL_CURL_VER_GE(7,37,0)
OPT_ENTRY( proxyheader,            PROXYHEADER,              LST, 0 )
#endif
OPT_ENTRY( http200aliases,         HTTP200ALIASES,           LST, 0 )
OPT_ENTRY( cookie,                 COOKIE,                   STR, LCURL_STORE_STRING )
OPT_ENTRY( cookiefile,             COOKIEFILE,               STR, LCURL_STORE_STRING )
OPT_ENTRY( cookiejar,              COOKIEJAR,                STR, LCURL_STORE_STRING )
OPT_ENTRY( cookiesession,          COOKIESESSION,            LNG, 0 )
OPT_ENTRY( cookielist,             COOKIELIST,               STR, LCURL_STORE_STRING )
OPT_ENTRY( httpget,                HTTPGET,                  LNG, 0 )
OPT_ENTRY( http_version,           HTTP_VERSION,             LNG, 0 )
OPT_ENTRY( ignore_content_length,  IGNORE_CONTENT_LENGTH,    LNG, 0 )
OPT_ENTRY( http_content_decoding,  HTTP_CONTENT_DECODING,    LNG, 0 )
OPT_ENTRY( http_transfer_decoding, HTTP_TRANSFER_DECODING,   LNG, 0 )
#if LCURL_CURL_VER_GE(7,36,0)
OPT_ENTRY( expect_100_timeout_ms,  EXPECT_100_TIMEOUT_MS,    LNG, 0 )
#endif

OPT_ENTRY( mail_from,              MAIL_FROM,                STR, LCURL_STORE_STRING )
OPT_ENTRY( mail_rcpt,              MAIL_RCPT,                STR, LCURL_STORE_STRING )
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( mail_auth,              MAIL_AUTH,                STR, LCURL_STORE_STRING )
#endif

OPT_ENTRY( tftp_blksize,           TFTP_BLKSIZE,             LNG, 0 )

OPT_ENTRY( ftpport,                 FTPPORT,                 STR, LCURL_STORE_STRING )
OPT_ENTRY( quote,                   QUOTE,                   LST, 0 )
OPT_ENTRY( postquote,               POSTQUOTE,               LST, 0 )
OPT_ENTRY( prequote,                PREQUOTE,                STR, LCURL_STORE_STRING )
OPT_ENTRY( dirlistonly,             DIRLISTONLY,             LNG, 0 )
OPT_ENTRY( append,                  APPEND,                  LNG, 0 )
OPT_ENTRY( ftp_use_eprt,            FTP_USE_EPRT,            LNG, 0 )
OPT_ENTRY( ftp_use_epsv,            FTP_USE_EPSV,            LNG, 0 )
OPT_ENTRY( ftp_use_pret,            FTP_USE_PRET,            LNG, 0 )
OPT_ENTRY( ftp_create_missing_dirs, FTP_CREATE_MISSING_DIRS, LNG, 0 )
OPT_ENTRY( ftp_response_timeout,    FTP_RESPONSE_TIMEOUT,    LNG, 0 )
OPT_ENTRY( ftp_alternative_to_user, FTP_ALTERNATIVE_TO_USER, STR, LCURL_STORE_STRING )
OPT_ENTRY( ftp_skip_pasv_ip,        FTP_SKIP_PASV_IP,        LNG, 0 )
OPT_ENTRY( ftpsslauth,              FTPSSLAUTH,              LNG, 0 )
OPT_ENTRY( ftp_ssl_ccc,             FTP_SSL_CCC,             LNG, 0 )
OPT_ENTRY( ftp_account,             FTP_ACCOUNT,             STR, LCURL_STORE_STRING )
OPT_ENTRY( ftp_filemethod,          FTP_FILEMETHOD,          LNG, 0 )

OPT_ENTRY( transfertext,            TRANSFERTEXT,            LNG, 0 )
OPT_ENTRY( proxy_transfer_mode,     PROXY_TRANSFER_MODE,     LNG, 0 )
OPT_ENTRY( crlf,                    CRLF,                    LNG, 0 )
OPT_ENTRY( range,                   RANGE,                   STR, LCURL_STORE_STRING )
OPT_ENTRY( resume_from,             RESUME_FROM,             LNG, 0 )
OPT_ENTRY( resume_from_large,       RESUME_FROM_LARGE,       LNG, 0 )
OPT_ENTRY( customrequest,           CUSTOMREQUEST,           STR, LCURL_STORE_STRING )
OPT_ENTRY( filetime,                FILETIME,                LNG, 0 )
OPT_ENTRY( nobody,                  NOBODY,                  LNG, 0 )
OPT_ENTRY( infilesize,              INFILESIZE,              LNG, 0 )
OPT_ENTRY( infilesize_large,        INFILESIZE_LARGE,        LNG, 0 )
OPT_ENTRY( upload,                  UPLOAD,                  LNG, 0 )
OPT_ENTRY( maxfilesize,             MAXFILESIZE,             LNG, 0 )
OPT_ENTRY( maxfilesize_large,       MAXFILESIZE_LARGE,       LNG, 0 )
OPT_ENTRY( timecondition,           TIMECONDITION,           LNG, 0 )
OPT_ENTRY( timevalue,               TIMEVALUE,               LNG, 0 )

OPT_ENTRY( timeout,                 TIMEOUT,                 LNG, 0 )
OPT_ENTRY( timeout_ms,              TIMEOUT_MS,              LNG, 0 )
OPT_ENTRY( low_speed_limit,         LOW_SPEED_LIMIT,         LNG, 0 )
OPT_ENTRY( low_speed_time,          LOW_SPEED_TIME,          LNG, 0 )
OPT_ENTRY( max_send_speed_large,    MAX_SEND_SPEED_LARGE,    LNG, 0 )
OPT_ENTRY( max_recv_speed_large,    MAX_RECV_SPEED_LARGE,    LNG, 0 )
OPT_ENTRY( maxconnects,             MAXCONNECTS,             LNG, 0 )
OPT_ENTRY( fresh_connect,           FRESH_CONNECT,           LNG, 0 )
OPT_ENTRY( forbid_reuse,            FORBID_REUSE,            LNG, 0 )
OPT_ENTRY( connecttimeout,          CONNECTTIMEOUT,          LNG, 0 )
OPT_ENTRY( connecttimeout_ms,       CONNECTTIMEOUT_MS,       LNG, 0 )
OPT_ENTRY( ipresolve,               IPRESOLVE,               LNG, 0 )
OPT_ENTRY( connect_only,            CONNECT_ONLY,            LNG, 0 )
OPT_ENTRY( use_ssl,                 USE_SSL,                 LNG, 0 )
OPT_ENTRY( resolve,                 RESOLVE,                 LST, 0 )
#if LCURL_CURL_VER_GE(7,33,0)
OPT_ENTRY( dns_interface,           DNS_INTERFACE,           STR, LCURL_STORE_STRING )
OPT_ENTRY( dns_local_ip4,           DNS_LOCAL_IP4,           STR, LCURL_STORE_STRING )
OPT_ENTRY( dns_local_ip6,           DNS_LOCAL_IP6,           STR, LCURL_STORE_STRING )
OPT_ENTRY( accepttimeout_ms,        ACCEPTTIMEOUT_MS,        LNG, 0 )
#endif

OPT_ENTRY( ssh_auth_types,          SSH_AUTH_TYPES,          LNG, 0)
OPT_ENTRY( ssh_host_public_key_md5, SSH_HOST_PUBLIC_KEY_MD5, STR, 0)
OPT_ENTRY( ssh_public_keyfile,      SSH_PUBLIC_KEYFILE,      STR, 0)
OPT_ENTRY( ssh_private_keyfile,     SSH_PRIVATE_KEYFILE,     STR, 0)
OPT_ENTRY( ssh_knownhosts,          SSH_KNOWNHOSTS,          STR, 0)

OPT_ENTRY( new_file_perms,          NEW_FILE_PERMS,          LNG, 0)
OPT_ENTRY( new_directory_perms,     NEW_DIRECTORY_PERMS,     LNG, 0)

OPT_ENTRY( telnetoptions,           TELNETOPTIONS,           LST, 0)

OPT_ENTRY(cainfo,              CAINFO,              STR, LCURL_STORE_STRING )
OPT_ENTRY(capath,              CAPATH,              STR, LCURL_STORE_STRING )
OPT_ENTRY(certinfo,            CERTINFO,            LNG, 0 )

OPT_ENTRY(sslcert,             SSLCERT,             STR, LCURL_STORE_STRING )
OPT_ENTRY(sslcerttype,         SSLCERTTYPE,         STR, LCURL_STORE_STRING )
OPT_ENTRY(sslengine,           SSLENGINE,           STR, LCURL_STORE_STRING )
OPT_ENTRY(sslengine_default,   SSLENGINE_DEFAULT,   LNG, 0 )
OPT_ENTRY(sslkey,              SSLKEY,              STR, LCURL_STORE_STRING )
OPT_ENTRY(sslkeytype,          SSLKEYTYPE,          STR, LCURL_STORE_STRING )
OPT_ENTRY(sslversion,          SSLVERSION,          LNG, 0 )
OPT_ENTRY(ssl_cipher_list,     SSL_CIPHER_LIST,     STR, LCURL_STORE_STRING )
// OPT_ENTRY(ssl_ctx_data,        SSL_CTX_DATA,        0 )  //! @todo
// OPT_ENTRY(ssl_ctx_function,    SSL_CTX_FUNCTION,    0 )  //! @todo
#if LCURL_CURL_VER_GE(7,33,0)
OPT_ENTRY(ssl_enable_alpn,     SSL_ENABLE_ALPN,     LNG, 0 )
OPT_ENTRY(ssl_enable_npn,      SSL_ENABLE_NPN,      LNG, 0 )
#endif
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY(ssl_options,         SSL_OPTIONS,         LNG, 0 )
#endif
OPT_ENTRY(ssl_sessionid_cache, SSL_SESSIONID_CACHE, LNG, 0 )
OPT_ENTRY(ssl_verifyhost,      SSL_VERIFYHOST,      LNG, 0 )
OPT_ENTRY(ssl_verifypeer,      SSL_VERIFYPEER,      LNG, 0 )


FLG_ENTRY( SSLVERSION_DEFAULT )
FLG_ENTRY( SSLVERSION_TLSv1   )
FLG_ENTRY( SSLVERSION_SSLv2   )
FLG_ENTRY( SSLVERSION_SSLv3   )
#if LCURL_CURL_VER_GE(7,34,0)
FLG_ENTRY( SSLVERSION_TLSv1_0 )
FLG_ENTRY( SSLVERSION_TLSv1_1 )
FLG_ENTRY( SSLVERSION_TLSv1_2 )
#endif


#ifdef OPT_ENTRY_IS_NULL
#  undef OPT_ENTRY
#endif

#ifdef FLG_ENTRY_IS_NULL
#  undef FLG_ENTRY
#endif
