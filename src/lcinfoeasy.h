OPT_ENTRY( effective_url,           EFFECTIVE_URL,           STR, 0)
OPT_ENTRY( response_code,           RESPONSE_CODE,           LNG, 0)
OPT_ENTRY( http_connectcode,        HTTP_CONNECTCODE,        LNG, 0)
OPT_ENTRY( filetime,                FILETIME,                LNG, 0)
OPT_ENTRY( total_time,              TOTAL_TIME,              DBL, 0)
OPT_ENTRY( namelookup_time,         NAMELOOKUP_TIME,         DBL, 0)
OPT_ENTRY( connect_time,            CONNECT_TIME,            DBL, 0)
OPT_ENTRY( appconnect_time,         APPCONNECT_TIME,         DBL, 0)
OPT_ENTRY( pretransfer_time,        PRETRANSFER_TIME,        DBL, 0)
OPT_ENTRY( starttransfer_time,      STARTTRANSFER_TIME,      DBL, 0)
OPT_ENTRY( redirect_time,           REDIRECT_TIME,           DBL, 0)
OPT_ENTRY( redirect_count,          REDIRECT_COUNT,          LNG, 0)
OPT_ENTRY( redirect_url,            REDIRECT_URL,            STR, 0)
OPT_ENTRY( size_upload,             SIZE_UPLOAD,             DBL, 0)
OPT_ENTRY( size_download,           SIZE_DOWNLOAD,           DBL, 0)
OPT_ENTRY( speed_download,          SPEED_DOWNLOAD,          DBL, 0)
OPT_ENTRY( speed_upload,            SPEED_UPLOAD,            DBL, 0)
OPT_ENTRY( header_size,             HEADER_SIZE,             LNG, 0)
OPT_ENTRY( request_size,            REQUEST_SIZE,            LNG, 0)
OPT_ENTRY( ssl_verifyresult,        SSL_VERIFYRESULT,        LNG, 0)
OPT_ENTRY( ssl_engines,             SSL_ENGINES,             LST, 0)
OPT_ENTRY( content_length_download, CONTENT_LENGTH_DOWNLOAD, DBL, 0)
OPT_ENTRY( content_length_upload,   CONTENT_LENGTH_UPLOAD,   DBL, 0)
OPT_ENTRY( content_type,            CONTENT_TYPE,            STR, 0)
OPT_ENTRY( httpauth_avail,          HTTPAUTH_AVAIL,          LNG, 0)
OPT_ENTRY( proxyauth_avail,         PROXYAUTH_AVAIL,         LNG, 0)
OPT_ENTRY( os_errno,                OS_ERRNO,                LNG, 0)
OPT_ENTRY( num_connects,            NUM_CONNECTS,            LNG, 0)
OPT_ENTRY( primary_ip,              PRIMARY_IP,              STR, 0)
OPT_ENTRY( certinfo,                CERTINFO,                CERTINFO, 0)
#if LCURL_CURL_VER_GE(7,21,0)
OPT_ENTRY( primary_port,            PRIMARY_PORT,            LNG, 0)
OPT_ENTRY( local_ip,                LOCAL_IP,                STR, 0)
OPT_ENTRY( local_port,              LOCAL_PORT,              LNG, 0)
#endif
OPT_ENTRY( cookielist,              COOKIELIST,              LST, 0)
OPT_ENTRY( lastsocket,              LASTSOCKET,              LNG, 0)
OPT_ENTRY( ftp_entry_path,          FTP_ENTRY_PATH,          STR, 0)
OPT_ENTRY( condition_unmet,         CONDITION_UNMET,         LNG, 0)
#if LCURL_CURL_VER_GE(7,20,0)
OPT_ENTRY( rtsp_session_id,         RTSP_SESSION_ID,         STR, 0)
OPT_ENTRY( rtsp_client_cseq,        RTSP_CLIENT_CSEQ,        LNG, 0)
OPT_ENTRY( rtsp_server_cseq,        RTSP_SERVER_CSEQ,        LNG, 0)
OPT_ENTRY( rtsp_cseq_recv,          RTSP_CSEQ_RECV,          LNG, 0)
#endif

#if LCURL_CURL_VER_GE(7,50,1)
OPT_ENTRY( http_version,            HTTP_VERSION,            LNG, 0)
#endif

#if LCURL_CURL_VER_GE(7,52,0)
OPT_ENTRY( proxy_ssl_verifyresult,  PROXY_SSL_VERIFYRESULT,  LNG, 0)
OPT_ENTRY( protocol,                PROTOCOL,                LNG, 0)
OPT_ENTRY( scheme,                  SCHEME,                  STR, 0)
#endif

#if LCURL_CURL_VER_GE(7,55,0)
OPT_ENTRY( content_length_download_t, CONTENT_LENGTH_DOWNLOAD_T, OFF, 0)
OPT_ENTRY( content_length_upload_t,   CONTENT_LENGTH_UPLOAD_T,   OFF, 0)
OPT_ENTRY( size_download_t,           SIZE_DOWNLOAD_T,           OFF, 0)
OPT_ENTRY( size_upload_t,             SIZE_UPLOAD_T,             OFF, 0)
OPT_ENTRY( speed_download_t,          SPEED_DOWNLOAD_T,          OFF, 0)
OPT_ENTRY( speed_upload_t,            SPEED_UPLOAD_T,            OFF, 0)
#endif

#if LCURL_CURL_VER_GE(7,59,0)
OPT_ENTRY( filetime_t,              FILETIME_T,              OFF, 0)
#endif

#if LCURL_CURL_VER_GE(7,61,0)
OPT_ENTRY(appconnect_time_t,        APPCONNECT_TIME_T,       OFF, 0)
OPT_ENTRY(connect_time_t,           CONNECT_TIME_T,          OFF, 0)
OPT_ENTRY(namelookup_time_t,        NAMELOOKUP_TIME_T,       OFF, 0)
OPT_ENTRY(pretransfer_time_t,       PRETRANSFER_TIME_T,      OFF, 0)
OPT_ENTRY(redirect_time_t,          REDIRECT_TIME_T,         OFF, 0)
OPT_ENTRY(starttransfer_time_t,     STARTTRANSFER_TIME_T,    OFF, 0)
OPT_ENTRY(total_time_t,             TOTAL_TIME_T,            OFF, 0)
#endif

#if LCURL_CURL_VER_GE(7,66,0)
OPT_ENTRY(retry_after,              RETRY_AFTER,             OFF, 0)
#endif

#if LCURL_CURL_VER_GE(7,72,0)
OPT_ENTRY(effective_method,         EFFECTIVE_METHOD,        STR, 0)
#endif

#if LCURL_CURL_VER_GE(7,73,0)
OPT_ENTRY(proxy_error,              PROXY_ERROR,             LNG, 0)
#endif

// OPT_ENTRY( PRIVATE,                 void     ) 
// OPT_ENTRY( TLS_SSL_PTR,             struct curl_tlssessioninfo **
// OPT_ENTRY( TLS_SESSION,             struct curl_tlssessioninfo *
