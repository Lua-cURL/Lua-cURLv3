
OPT_ENTRY(pipelining,                  PIPELINING,                    LNG,     0 )
OPT_ENTRY(maxconnects,                 MAXCONNECTS,                   LNG,     0 )

#if LCURL_CURL_VER_GE(7,30,0)
OPT_ENTRY(max_host_connections,        MAX_HOST_CONNECTIONS,          LNG,     0 )
OPT_ENTRY(max_pipeline_length,         MAX_PIPELINE_LENGTH,           LNG,     0 )
OPT_ENTRY(content_length_penalty_size, CONTENT_LENGTH_PENALTY_SIZE,   LNG,     0 )
OPT_ENTRY(chunk_length_penalty_size,   CHUNK_LENGTH_PENALTY_SIZE,     LNG,     0 )
OPT_ENTRY(pipelining_site_bl,          PIPELINING_SITE_BL,            STR_ARR, 0 )
OPT_ENTRY(pipelining_server_bl,        PIPELINING_SERVER_BL,          STR_ARR, 0 )
OPT_ENTRY(max_total_connections,       MAX_TOTAL_CONNECTIONS,         LNG,     0 )
#endif

#if LCURL_CURL_VER_GE(7,67,0)
OPT_ENTRY(max_concurrent_streams,       MAX_CONCURRENT_STREAMS,         LNG,     0 )
#endif
