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
#  define OPT_ENTRY(a,b,c,d,e)
#  define OPT_ENTRY_IS_NULL
#endif

#ifndef FLG_ENTRY
#  define FLG_ENTRY(a)
#  define FLG_ENTRY_IS_NULL
#endif

#ifndef LCURL_DEFAULT_VALUE
#  define LCURL_DEFAULT_VALUE 0
#endif

//{ Reset system macros

#ifdef TCP_FASTOPEN
#  define LCURL__TCP_FASTOPEN TCP_FASTOPEN
#  undef TCP_FASTOPEN
#endif

#ifdef TCP_KEEPIDLE
#  define LCURL__TCP_KEEPIDLE TCP_KEEPIDLE
#  undef TCP_KEEPIDLE
#endif

#ifdef TCP_KEEPINTVL
#  define LCURL__TCP_KEEPINTVL TCP_KEEPINTVL
#  undef TCP_KEEPINTVL
#endif

#ifdef TCP_NODELAY
#  define LCURL__TCP_NODELAY TCP_NODELAY
#  undef TCP_NODELAY
#endif

#ifdef TCP_KEEPALIVE
#  define LCURL__TCP_KEEPALIVE TCP_KEEPALIVE
#  undef TCP_KEEPALIVE
#endif

#ifdef BUFFERSIZE
#  define LCURL__BUFFERSIZE BUFFERSIZE
#  undef BUFFERSIZE
#endif

#ifdef INTERFACE
#  define LCURL__INTERFACE INTERFACE
#  undef INTERFACE
#endif

//}

OPT_ENTRY( verbose,                VERBOSE,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( header,                 HEADER,                   LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( noprogress,             NOPROGRESS,               LNG, 0,                  1 )
OPT_ENTRY( nosignal,               NOSIGNAL,                 LNG, 0,                  LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,21,0)
OPT_ENTRY( wildcardmatch,          WILDCARDMATCH,            LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif

OPT_ENTRY( url,                    URL,                      STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( failonerror,            FAILONERROR,              LNG, 0,                  LCURL_DEFAULT_VALUE )

OPT_ENTRY( protocols,              PROTOCOLS,                LNG, 0,                  CURLPROTO_ALL )
OPT_ENTRY( redir_protocols,        REDIR_PROTOCOLS,          LNG, 0,                  CURLPROTO_ALL ) /*! @fixme All protocols except for FILE and SCP */
OPT_ENTRY( proxy,                  PROXY,                    STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( proxyport,              PROXYPORT,                LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( proxytype,              PROXYTYPE,                LNG, 0,                  CURLPROXY_HTTP )
OPT_ENTRY( noproxy,                NOPROXY,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( httpproxytunnel,        HTTPPROXYTUNNEL,          LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( socks5_gssapi_service,  SOCKS5_GSSAPI_SERVICE,    STR, LCURL_STORE_STRING, "rcmd/server-fqdn" )
OPT_ENTRY( socks5_gssapi_nec,      SOCKS5_GSSAPI_NEC,        LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @check doc says nothing */
OPT_ENTRY( interface,              INTERFACE,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( localport,              LOCALPORT,                LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( localportrange,         LOCALPORTRANGE,           LNG, 0,                  1 )
OPT_ENTRY( dns_cache_timeout,      DNS_CACHE_TIMEOUT,        LNG, 0,                  60 )

#if !LCURL_CURL_VER_GE(7,65,0)
OPT_ENTRY( dns_use_global_cache,   DNS_USE_GLOBAL_CACHE,     LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif

#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( dns_servers,            DNS_SERVERS,              STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( buffersize,             BUFFERSIZE,               LNG, 0,                  CURL_MAX_WRITE_SIZE )
OPT_ENTRY( port,                   PORT,                     LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( tcp_nodelay,            TCP_NODELAY,              LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( address_scope,          ADDRESS_SCOPE,            LNG, 0,                  LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( tcp_keepalive,          TCP_KEEPALIVE,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( tcp_keepidle,           TCP_KEEPIDLE,             LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @check doc says nothing */
OPT_ENTRY( tcp_keepintvl,          TCP_KEEPINTVL,            LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @check doc says nothing */
#endif

OPT_ENTRY( netrc,                  NETRC,                    LNG, 0,                  CURL_NETRC_IGNORED )
OPT_ENTRY( netrc_file,             NETRC_FILE,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( userpwd,                USERPWD,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( proxyuserpwd,           PROXYUSERPWD,             STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( username,               USERNAME,                 STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( password,               PASSWORD,                 STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,31,0)
OPT_ENTRY( login_options,          LOGIN_OPTIONS,            STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( proxyusername,          PROXYUSERNAME,            STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( proxypassword,          PROXYPASSWORD,            STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( httpauth,               HTTPAUTH,                 LNG, 0,                  CURLAUTH_BASIC      )
#if LCURL_CURL_VER_GE(7,21,4)
OPT_ENTRY( tlsauth_username,       TLSAUTH_USERNAME,         STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( tlsauth_password,       TLSAUTH_PASSWORD,         STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( tlsauth_type,           TLSAUTH_TYPE,             STR, 0,                  "" )
#endif
OPT_ENTRY( proxyauth,              PROXYAUTH,                LNG, 0,                  CURLAUTH_BASIC )
#if LCURL_CURL_VER_GE(7,31,0)
OPT_ENTRY( sasl_ir,                SASL_IR,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,33,0)
OPT_ENTRY( xoauth2_bearer,         XOAUTH2_BEARER,           STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#endif

OPT_ENTRY( autoreferer,            AUTOREFERER,              LNG, 0,                  LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,21,6)
OPT_ENTRY( accept_encoding,        ACCEPT_ENCODING,          STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( transfer_encoding,      TRANSFER_ENCODING,        LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( followlocation,         FOLLOWLOCATION,           LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( unrestricted_auth,      UNRESTRICTED_AUTH,        LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( maxredirs,              MAXREDIRS,                LNG, 0,                  -1 )
OPT_ENTRY( postredir,              POSTREDIR,                LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( put,                    PUT,                      LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( post,                   POST,                     LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( referer,                REFERER,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( useragent,              USERAGENT,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,37,0)
OPT_ENTRY( headeropt,              HEADEROPT,                LNG, 0,                  CURLHEADER_UNIFIED )
#endif
OPT_ENTRY( httpheader,             HTTPHEADER,               LST, 0,                  LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,37,0)
OPT_ENTRY( proxyheader,            PROXYHEADER,              LST, 0,                  LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( http200aliases,         HTTP200ALIASES,           LST, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( cookie,                 COOKIE,                   STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( cookiefile,             COOKIEFILE,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( cookiejar,              COOKIEJAR,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( cookiesession,          COOKIESESSION,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( cookielist,             COOKIELIST,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( httpget,                HTTPGET,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( http_version,           HTTP_VERSION,             LNG, 0,                  CURL_HTTP_VERSION_NONE )
OPT_ENTRY( ignore_content_length,  IGNORE_CONTENT_LENGTH,    LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( http_content_decoding,  HTTP_CONTENT_DECODING,    LNG, 0,                  1 )
OPT_ENTRY( http_transfer_decoding, HTTP_TRANSFER_DECODING,   LNG, 0,                  1 )
#if LCURL_CURL_VER_GE(7,36,0)
OPT_ENTRY( expect_100_timeout_ms,  EXPECT_100_TIMEOUT_MS,    LNG, 0,                  1000 )
#endif

#if LCURL_CURL_VER_GE(7,20,0)
OPT_ENTRY( mail_from,              MAIL_FROM,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE ) /*! @check doc says `blank` */
OPT_ENTRY( mail_rcpt,              MAIL_RCPT,                LST, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( mail_auth,              MAIL_AUTH,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#endif

OPT_ENTRY( tftp_blksize,           TFTP_BLKSIZE,             LNG, 0,                  512 )

OPT_ENTRY( ftpport,                 FTPPORT,                 STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( quote,                   QUOTE,                   LST, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( postquote,               POSTQUOTE,               LST, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( prequote,                PREQUOTE,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( dirlistonly,             DIRLISTONLY,             LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( append,                  APPEND,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( ftp_use_eprt,            FTP_USE_EPRT,            LNG, 0,                  LCURL_DEFAULT_VALUE )/*! @check doc says nothing */
OPT_ENTRY( ftp_use_epsv,            FTP_USE_EPSV,            LNG, 0,                  1 )
#if LCURL_CURL_VER_GE(7,20,0)
OPT_ENTRY( ftp_use_pret,            FTP_USE_PRET,            LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( ftp_create_missing_dirs, FTP_CREATE_MISSING_DIRS, LNG, 0,                  CURLFTP_CREATE_DIR_NONE )
OPT_ENTRY( ftp_response_timeout,    FTP_RESPONSE_TIMEOUT,    LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @fixme doc says `None` */
OPT_ENTRY( ftp_alternative_to_user, FTP_ALTERNATIVE_TO_USER, STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( ftp_skip_pasv_ip,        FTP_SKIP_PASV_IP,        LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( ftpsslauth,              FTPSSLAUTH,              LNG, 0,                  CURLFTPAUTH_DEFAULT )
OPT_ENTRY( ftp_ssl_ccc,             FTP_SSL_CCC,             LNG, 0,                  CURLFTPSSL_CCC_NONE )
OPT_ENTRY( ftp_account,             FTP_ACCOUNT,             STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( ftp_filemethod,          FTP_FILEMETHOD,          LNG, 0,                  CURLFTPMETHOD_MULTICWD )

OPT_ENTRY( transfertext,            TRANSFERTEXT,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( proxy_transfer_mode,     PROXY_TRANSFER_MODE,     LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( crlf,                    CRLF,                    LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( range,                   RANGE,                   STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( resume_from,             RESUME_FROM,             LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( resume_from_large,       RESUME_FROM_LARGE,       LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( customrequest,           CUSTOMREQUEST,           STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( filetime,                FILETIME,                LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( nobody,                  NOBODY,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( infilesize,              INFILESIZE,              LNG, 0,                  LCURL_DEFAULT_VALUE )/*! @fixme doc says `Unset` */
OPT_ENTRY( infilesize_large,        INFILESIZE_LARGE,        LNG, 0,                  LCURL_DEFAULT_VALUE )/*! @fixme doc says `Unset` */
OPT_ENTRY( upload,                  UPLOAD,                  LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( maxfilesize,             MAXFILESIZE,             LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @fixme doc says `None` */
OPT_ENTRY( maxfilesize_large,       MAXFILESIZE_LARGE,       LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @fixme doc says `None` */
OPT_ENTRY( timecondition,           TIMECONDITION,           LNG, 0,                  CURL_TIMECOND_NONE )
OPT_ENTRY( timevalue,               TIMEVALUE,               LNG, 0,                  LCURL_DEFAULT_VALUE )

OPT_ENTRY( timeout,                 TIMEOUT,                 LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( timeout_ms,              TIMEOUT_MS,              LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( low_speed_limit,         LOW_SPEED_LIMIT,         LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( low_speed_time,          LOW_SPEED_TIME,          LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( max_send_speed_large,    MAX_SEND_SPEED_LARGE,    LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( max_recv_speed_large,    MAX_RECV_SPEED_LARGE,    LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( maxconnects,             MAXCONNECTS,             LNG, 0,                  5 )
OPT_ENTRY( fresh_connect,           FRESH_CONNECT,           LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( forbid_reuse,            FORBID_REUSE,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( connecttimeout,          CONNECTTIMEOUT,          LNG, 0,                  300 )
OPT_ENTRY( connecttimeout_ms,       CONNECTTIMEOUT_MS,       LNG, 0,                  300000 )
OPT_ENTRY( ipresolve,               IPRESOLVE,               LNG, 0,                  CURL_IPRESOLVE_WHATEVER )
OPT_ENTRY( connect_only,            CONNECT_ONLY,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( use_ssl,                 USE_SSL,                 LNG, 0,                  CURLUSESSL_NONE )
#if LCURL_CURL_VER_GE(7,21,3)
OPT_ENTRY( resolve,                 RESOLVE,                 LST, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,33,0)
OPT_ENTRY( dns_interface,           DNS_INTERFACE,           STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( dns_local_ip4,           DNS_LOCAL_IP4,           STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( dns_local_ip6,           DNS_LOCAL_IP6,           STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( accepttimeout_ms,        ACCEPTTIMEOUT_MS,        LNG, 0,                  60000 )
#endif

OPT_ENTRY( ssh_auth_types,          SSH_AUTH_TYPES,          LNG, 0,                  LCURL_DEFAULT_VALUE) /*! @fixme doc says `None` */
OPT_ENTRY( ssh_host_public_key_md5, SSH_HOST_PUBLIC_KEY_MD5, STR, 0,                  LCURL_DEFAULT_VALUE)
OPT_ENTRY( ssh_public_keyfile,      SSH_PUBLIC_KEYFILE,      STR, 0,                  LCURL_DEFAULT_VALUE)
OPT_ENTRY( ssh_private_keyfile,     SSH_PRIVATE_KEYFILE,     STR, 0,                  LCURL_DEFAULT_VALUE)
OPT_ENTRY( ssh_knownhosts,          SSH_KNOWNHOSTS,          STR, 0,                  LCURL_DEFAULT_VALUE)

OPT_ENTRY( new_file_perms,          NEW_FILE_PERMS,          LNG, 0,                  0644)
OPT_ENTRY( new_directory_perms,     NEW_DIRECTORY_PERMS,     LNG, 0,                  0755)

OPT_ENTRY( telnetoptions,           TELNETOPTIONS,           LST, 0,                  LCURL_DEFAULT_VALUE)

OPT_ENTRY( random_file,             RANDOM_FILE,             STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( egdsocket,               EGDSOCKET,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( issuercert,              ISSUERCERT,              STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( krblevel,                KRBLEVEL,                STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )

OPT_ENTRY( cainfo,                  CAINFO,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE ) /*! @fixme doc says `Built-in system specific` */
OPT_ENTRY( capath,                  CAPATH,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( certinfo,                CERTINFO,                LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( crlfile,                 CRLFILE,                 STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )

OPT_ENTRY( sslcert,                 SSLCERT,                 STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( sslcerttype,             SSLCERTTYPE,             STR, LCURL_STORE_STRING, "PEM"               )
OPT_ENTRY( sslengine,               SSLENGINE,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( sslengine_default,       SSLENGINE_DEFAULT,       LNG, 0,                  LCURL_DEFAULT_VALUE ) /*! @fixme doc says `None` */
OPT_ENTRY( sslkey,                  SSLKEY,                  STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( sslkeytype,              SSLKEYTYPE,              STR, LCURL_STORE_STRING, "PEM"               )
OPT_ENTRY( sslversion,              SSLVERSION,              LNG, 0,                  CURL_SSLVERSION_DEFAULT )
OPT_ENTRY( ssl_cipher_list,         SSL_CIPHER_LIST,         STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#if LCURL_CURL_VER_GE(7,36,0)
OPT_ENTRY( ssl_enable_alpn,         SSL_ENABLE_ALPN,         LNG, 0,                  1 )
OPT_ENTRY( ssl_enable_npn,          SSL_ENABLE_NPN,          LNG, 0,                  1 )
#endif
#if LCURL_CURL_VER_GE(7,25,0)
OPT_ENTRY( ssl_options,             SSL_OPTIONS,             LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
OPT_ENTRY( ssl_sessionid_cache,     SSL_SESSIONID_CACHE,     LNG, 0,                  1 )
OPT_ENTRY( ssl_verifyhost,          SSL_VERIFYHOST,          LNG, 0,                  2 )
OPT_ENTRY( ssl_verifypeer,          SSL_VERIFYPEER,          LNG, 0,                  1 )
OPT_ENTRY( keypasswd,               KEYPASSWD,               STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )

#if LCURL_CURL_VER_GE(7,20,0)
OPT_ENTRY( rtsp_client_cseq,        RTSP_CLIENT_CSEQ,        LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( rtsp_request,            RTSP_REQUEST,            LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( rtsp_server_cseq,        RTSP_SERVER_CSEQ,        LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( rtsp_session_id,         RTSP_SESSION_ID,         STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( rtsp_stream_uri,         RTSP_STREAM_URI,         STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
OPT_ENTRY( rtsp_transport,          RTSP_TRANSPORT,          STR, LCURL_STORE_STRING, LCURL_DEFAULT_VALUE )
#endif

#if LCURL_CURL_VER_GE(7,22,0)
OPT_ENTRY( gssapi_delegation,       GSSAPI_DELEGATION,        LNG, 0,                  CURLGSSAPI_DELEGATION_NONE )
#endif 

FLG_ENTRY( SSLVERSION_DEFAULT )
FLG_ENTRY( SSLVERSION_TLSv1   )
FLG_ENTRY( SSLVERSION_SSLv2   )
FLG_ENTRY( SSLVERSION_SSLv3   )
#if LCURL_CURL_VER_GE(7,34,0)
FLG_ENTRY( SSLVERSION_TLSv1_0 )
FLG_ENTRY( SSLVERSION_TLSv1_1 )
FLG_ENTRY( SSLVERSION_TLSv1_2 )
#endif
#if LCURL_CURL_VER_GE(7,52,0)
FLG_ENTRY( SSLVERSION_TLSv1_3 )
#endif

#if LCURL_CURL_VER_GE(7,54,0)
FLG_ENTRY( SSLVERSION_MAX_NONE    )
FLG_ENTRY( SSLVERSION_MAX_DEFAULT )
FLG_ENTRY( SSLVERSION_MAX_TLSv1_0 )
FLG_ENTRY( SSLVERSION_MAX_TLSv1_1 )
FLG_ENTRY( SSLVERSION_MAX_TLSv1_2 )
FLG_ENTRY( SSLVERSION_MAX_TLSv1_3 )
#endif

#if LCURL_CURL_VER_GE(7,21,4)
FLG_ENTRY( TLSAUTH_SRP )
#endif

FLG_ENTRY( HTTP_VERSION_NONE  )
FLG_ENTRY( HTTP_VERSION_1_0   )
FLG_ENTRY( HTTP_VERSION_1_1   )
#if LCURL_CURL_VER_GE(7,33,0)
FLG_ENTRY( HTTP_VERSION_2_0   )
#endif
#if LCURL_CURL_VER_GE(7,43,0)
FLG_ENTRY( HTTP_VERSION_2     )
#endif
#if LCURL_CURL_VER_GE(7,47,0)
FLG_ENTRY( HTTP_VERSION_2TLS  )
#endif
#if LCURL_CURL_VER_GE(7,49,0)
FLG_ENTRY( HTTP_VERSION_2_PRIOR_KNOWLEDGE )
#endif
#if LCURL_CURL_VER_GE(7,66,0)
FLG_ENTRY( HTTP_VERSION_3 )
#endif

FLG_ENTRY( READFUNC_PAUSE     ) /*7.18.0*/
FLG_ENTRY( WRITEFUNC_PAUSE    ) /*7.18.0*/

FLG_ENTRY( POLL_IN            ) /*7.14.0*/
FLG_ENTRY( POLL_INOUT         ) /*7.14.0*/
FLG_ENTRY( POLL_NONE          ) /*7.14.0*/
FLG_ENTRY( POLL_OUT           ) /*7.14.0*/
FLG_ENTRY( POLL_REMOVE        ) /*7.14.0*/
FLG_ENTRY( SOCKET_TIMEOUT     ) /*7.14.0*/

FLG_ENTRY( CSELECT_ERR        ) /*7.16.3*/
FLG_ENTRY( CSELECT_IN         ) /*7.16.3*/
FLG_ENTRY( CSELECT_OUT        ) /*7.16.3*/

FLG_ENTRY( IPRESOLVE_WHATEVER ) /*7.10.8*/
FLG_ENTRY( IPRESOLVE_V4       ) /*7.10.8*/
FLG_ENTRY( IPRESOLVE_V6       ) /*7.10.8*/

#if LCURL_CURL_VER_GE(7,20,0)
FLG_ENTRY( RTSPREQ_OPTIONS       )
FLG_ENTRY( RTSPREQ_DESCRIBE      )
FLG_ENTRY( RTSPREQ_ANNOUNCE      )
FLG_ENTRY( RTSPREQ_SETUP         )
FLG_ENTRY( RTSPREQ_PLAY          )
FLG_ENTRY( RTSPREQ_PAUSE         )
FLG_ENTRY( RTSPREQ_TEARDOWN      )
FLG_ENTRY( RTSPREQ_GET_PARAMETER )
FLG_ENTRY( RTSPREQ_SET_PARAMETER )
FLG_ENTRY( RTSPREQ_RECORD        )
FLG_ENTRY( RTSPREQ_RECEIVE       )
#endif

#if LCURL_CURL_VER_GE(7,39,0)
OPT_ENTRY( pinnedpublickey,    PINNEDPUBLICKEY,    STR, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,40,0)
OPT_ENTRY( unix_socket_path,   UNIX_SOCKET_PATH,   STR, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,41,0)
OPT_ENTRY( ssl_verifystatus,   SSL_VERIFYSTATUS,   LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,42,0)
OPT_ENTRY( ssl_falsestart,     SSL_FALSESTART,     LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( path_as_is,         PATH_AS_IS,         LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,43,0)
OPT_ENTRY( proxy_service_name, PROXY_SERVICE_NAME, STR, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( service_name,       SERVICE_NAME,       STR, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( pipewait,           PIPEWAIT,           LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,45,0)
OPT_ENTRY( default_protocol,   DEFAULT_PROTOCOL,   STR, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,46,0)
OPT_ENTRY( stream_weight,      STREAM_WEIGHT,      LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,48,0)
OPT_ENTRY( tftp_no_options,    TFTP_NO_OPTIONS,    LNG, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,49,0)
OPT_ENTRY( tcp_fastopen,       TCP_FASTOPEN,       LNG, 0,                  LCURL_DEFAULT_VALUE )
OPT_ENTRY( connect_to,         CONNECT_TO,         LST, 0,                  LCURL_DEFAULT_VALUE )
#endif
#if LCURL_CURL_VER_GE(7,51,0)
OPT_ENTRY( keep_sending_on_error, KEEP_SENDING_ON_ERROR, LNG, 0,            LCURL_DEFAULT_VALUE )
#endif

#if LCURL_CURL_VER_GE(7,52,0)
OPT_ENTRY( proxy_cainfo,           PROXY_CAINFO,           STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_capath,           PROXY_CAPATH,           STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_ssl_verifypeer,   PROXY_SSL_VERIFYPEER,   LNG, 0, 1)
OPT_ENTRY( proxy_ssl_verifyhost,   PROXY_SSL_VERIFYHOST,   LNG, 0, 2)
OPT_ENTRY( proxy_sslversion,       PROXY_SSLVERSION,       LNG, 0, CURL_SSLVERSION_DEFAULT)
OPT_ENTRY( proxy_tlsauth_username, PROXY_TLSAUTH_USERNAME, STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_tlsauth_password, PROXY_TLSAUTH_PASSWORD, STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_tlsauth_type,     PROXY_TLSAUTH_TYPE,     STR, 0, "")
OPT_ENTRY( proxy_sslcert,          PROXY_SSLCERT,          STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_sslcerttype,      PROXY_SSLCERTTYPE,      STR, 0, "PEM")
OPT_ENTRY( proxy_sslkey,           PROXY_SSLKEY,           STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_sslkeytype,       PROXY_SSLKEYTYPE,       STR, 0, "PEM") /* default value not defined. Use same as for `SSLKEYTYPE` */
OPT_ENTRY( proxy_keypasswd,        PROXY_KEYPASSWD,        STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_ssl_cipher_list,  PROXY_SSL_CIPHER_LIST,  STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_crlfile,          PROXY_CRLFILE,          STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_ssl_options,      PROXY_SSL_OPTIONS,      LNG, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( pre_proxy,              PRE_PROXY,              STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( proxy_pinnedpublickey,  PROXY_PINNEDPUBLICKEY,  STR, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,53,0)
OPT_ENTRY( abstract_unix_socket,   ABSTRACT_UNIX_SOCKET,  STR, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,54,0)
OPT_ENTRY( suppress_connect_headers, SUPPRESS_CONNECT_HEADERS, LNG, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,55,0)
OPT_ENTRY( request_target,           REQUEST_TARGET,           STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY( socks5_auth,              SOCKS5_AUTH,              LNG, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,56,0)
OPT_ENTRY( ssh_compression,          SSH_COMPRESSION,          LNG, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,59,0)
OPT_ENTRY( happy_eyeballs_timeout_ms,HAPPY_EYEBALLS_TIMEOUT_MS,LNG, 0, CURL_HET_DEFAULT)
OPT_ENTRY( timevalue_large,          TIMEVALUE_LARGE          ,OFF, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,60,0)
OPT_ENTRY(dns_shuffle_addresses, DNS_SHUFFLE_ADDRESSES, LNG, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY(haproxyprotocol,       HAPROXYPROTOCOL,       LNG, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,61,0)
OPT_ENTRY(disallow_username_in_url, DISALLOW_USERNAME_IN_URL, LNG, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY(proxy_tls13_ciphers,      PROXY_TLS13_CIPHERS,      STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY(tls13_ciphers,            TLS13_CIPHERS,            STR, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,62,0)
OPT_ENTRY(upkeep_interval_ms,       UPKEEP_INTERVAL_MS,       LNG, 0, CURL_UPKEEP_INTERVAL_DEFAULT)
OPT_ENTRY(doh_url,                  DOH_URL,                  STR, 0, LCURL_DEFAULT_VALUE)
// thre no named value for default value. It just defined as 64kB in documentation
OPT_ENTRY(upload_buffersize,        UPLOAD_BUFFERSIZE,        LNG, 0, 64 * 1024)
#endif

#if LCURL_CURL_VER_GE(7,64,0)
OPT_ENTRY(http09_allowed,           HTTP09_ALLOWED,           LNG, 0, 0)
#endif

#if LCURL_CURL_VER_GE(7,64,1)
OPT_ENTRY(altsvc,                   ALTSVC,                   STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY(altsvc_ctrl,              ALTSVC_CTRL,              LNG, 0, 0)
#endif

#if LCURL_CURL_VER_GE(7,65,0)
OPT_ENTRY(maxage_conn,              MAXAGE_CONN,              LNG, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,66,0)
OPT_ENTRY(sasl_authzid,            SASL_AUTHZID,              STR, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,68,0)
FLG_ENTRY( PROGRESSFUNC_CONTINUE )
#endif

#if LCURL_CURL_VER_GE(7,69,0)
OPT_ENTRY(mail_rcpt_alllowfails,   MAIL_RCPT_ALLLOWFAILS,     LNG, 0, 1)
#endif

#if LCURL_CURL_VER_GE(7,71,0)
OPT_ENTRY(sslcert_blob,       SSLCERT_BLOB,       BLB, 0, 0)
OPT_ENTRY(sslkey_blob,        SSLKEY_BLOB,        BLB, 0, 0)
OPT_ENTRY(proxy_sslcert_blob, PROXY_SSLCERT_BLOB, BLB, 0, 0)
OPT_ENTRY(proxy_sslkey_blob,  PROXY_SSLKEY_BLOB,  BLB, 0, 0)
OPT_ENTRY(issuercert_blob,    ISSUERCERT_BLOB,    BLB, 0, 0)

OPT_ENTRY(proxy_issuercert,      PROXY_ISSUERCERT,      STR, 0, LCURL_DEFAULT_VALUE)
OPT_ENTRY(proxy_issuercert_blob, PROXY_ISSUERCERT_BLOB, BLB, 0, 0)
#endif

#if LCURL_CURL_VER_GE(7,73,0)
OPT_ENTRY(ssl_ec_curves,      SSL_EC_CURVES,      STR, 0, LCURL_DEFAULT_VALUE)
#endif

#if LCURL_CURL_VER_GE(7,74,0) && LCURL_USE_HSTS
OPT_ENTRY(hsts_ctrl,          HSTS_CTRL,          LNG, 0, 0)
OPT_ENTRY(hsts,               HSTS,               STR, 0, LCURL_DEFAULT_VALUE)
#endif

//{ Restore system macros

#ifdef LCURL__TCP_FASTOPEN
#  define TCP_FASTOPEN LCURL__TCP_FASTOPEN
#  undef LCURL__TCP_FASTOPEN
#endif

#ifdef LCURL__TCP_KEEPIDLE
#  define TCP_KEEPIDLE LCURL__TCP_KEEPIDLE
#  undef LCURL__TCP_KEEPIDLE
#endif

#ifdef LCURL__TCP_KEEPINTVL
#  define TCP_KEEPINTVL LCURL__TCP_KEEPINTVL
#  undef LCURL__TCP_KEEPINTVL
#endif

#ifdef LCURL__TCP_NODELAY
#  define TCP_NODELAY LCURL__TCP_NODELAY
#  undef LCURL__TCP_NODELAY
#endif

#ifdef LCURL__TCP_KEEPALIVE
#  define TCP_KEEPALIVE LCURL__TCP_KEEPALIVE
#  undef LCURL__TCP_KEEPALIVE
#endif

#ifdef LCURL__BUFFERSIZE
#  define BUFFERSIZE LCURL__BUFFERSIZE
#  undef LCURL__BUFFERSIZE
#endif

#ifdef LCURL__INTERFACE
#  define INTERFACE LCURL__INTERFACE
#  undef LCURL__INTERFACE
#endif

//}

#ifdef OPT_ENTRY_IS_NULL
#  undef OPT_ENTRY
#endif

#ifdef FLG_ENTRY_IS_NULL
#  undef FLG_ENTRY
#endif
