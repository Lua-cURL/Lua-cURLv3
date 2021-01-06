/* Bitmasks for CURLOPT_HTTPAUTH and CURLOPT_PROXYAUTH options */
FLG_ENTRY(AUTH_NONE                       )
FLG_ENTRY(AUTH_BASIC                      )
FLG_ENTRY(AUTH_DIGEST                     )
FLG_ENTRY(AUTH_GSSNEGOTIATE               )
#if LCURL_CURL_VER_GE(7,38,0)
FLG_ENTRY(AUTH_NEGOTIATE                  )
#endif
FLG_ENTRY(AUTH_NTLM                       )
#if LCURL_CURL_VER_GE(7,19,3)
FLG_ENTRY(AUTH_DIGEST_IE                  )
#endif
#if LCURL_CURL_VER_GE(7,22,0)
FLG_ENTRY(AUTH_NTLM_WB                    )
#endif
#if LCURL_CURL_VER_GE(7,21,3)
FLG_ENTRY(AUTH_ONLY                       )
#endif
FLG_ENTRY(AUTH_ANY                        )
FLG_ENTRY(AUTH_ANYSAFE                    )
#if LCURL_CURL_VER_GE(7,55,0)
FLG_ENTRY(AUTH_GSSAPI                     )
#endif
#if LCURL_CURL_VER_GE(7,61,0)
FLG_ENTRY(AUTH_BEARER                     )
#endif

#ifdef CURLSSH_AUTH_ANY
FLG_ENTRY(SSH_AUTH_ANY                    )
#endif
#ifdef CURLSSH_AUTH_NONE
FLG_ENTRY(SSH_AUTH_NONE                   )
#endif
#ifdef CURLSSH_AUTH_PUBLICKEY
FLG_ENTRY(SSH_AUTH_PUBLICKEY              )
#endif
#ifdef CURLSSH_AUTH_PASSWORD
FLG_ENTRY(SSH_AUTH_PASSWORD               )
#endif
#ifdef CURLSSH_AUTH_HOST
FLG_ENTRY(SSH_AUTH_HOST                   )
#endif
#ifdef CURLSSH_AUTH_GSSAPI
FLG_ENTRY(SSH_AUTH_GSSAPI                 )
#endif
#ifdef CURLSSH_AUTH_KEYBOARD
FLG_ENTRY(SSH_AUTH_KEYBOARD               )
#endif
#ifdef CURLSSH_AUTH_AGENT
FLG_ENTRY(SSH_AUTH_AGENT                  )
#endif
#ifdef CURLSSH_AUTH_DEFAULT
FLG_ENTRY(SSH_AUTH_DEFAULT                )
#endif

#ifdef CURLGSSAPI_DELEGATION_NONE
FLG_ENTRY(GSSAPI_DELEGATION_NONE          )
#endif
#ifdef CURLGSSAPI_DELEGATION_POLICY_FLAG
FLG_ENTRY(GSSAPI_DELEGATION_POLICY_FLAG   )
#endif
#ifdef CURLGSSAPI_DELEGATION_FLAG
FLG_ENTRY(GSSAPI_DELEGATION_FLAG          )
#endif

/* Bitmasks for CURLOPT_HTTPAUTH and CURLOPT_PROXYAUTH options */
FLG_ENTRY(USESSL_NONE                     )
FLG_ENTRY(USESSL_TRY                      )
FLG_ENTRY(USESSL_CONTROL                  )
FLG_ENTRY(USESSL_ALL                      )

/* Definition of bits for the CURLOPT_SSL_OPTIONS argument: */
#ifdef CURLSSLOPT_ALLOW_BEAST
FLG_ENTRY(SSLOPT_ALLOW_BEAST              )
#endif
#ifdef CURLSSLOPT_NO_REVOKE
FLG_ENTRY(SSLOPT_NO_REVOKE                )
#endif
#ifdef CURLSSLOPT_NO_PARTIALCHAIN
FLG_ENTRY(SSLOPT_NO_PARTIALCHAIN          )
#endif
#ifdef CURLSSLOPT_REVOKE_BEST_EFFORT
FLG_ENTRY(SSLOPT_REVOKE_BEST_EFFORT       )
#endif
#ifdef CURLSSLOPT_NATIVE_CA
FLG_ENTRY(SSLOPT_NATIVE_CA                )
#endif

/* parameter for the CURLOPT_FTP_SSL_CCC option */
FLG_ENTRY(FTPSSL_CCC_NONE                 )
FLG_ENTRY(FTPSSL_CCC_PASSIVE              )
FLG_ENTRY(FTPSSL_CCC_ACTIVE               )

/* parameter for the CURLOPT_FTPSSLAUTH option */
FLG_ENTRY(FTPAUTH_DEFAULT                 )
FLG_ENTRY(FTPAUTH_SSL                     )
FLG_ENTRY(FTPAUTH_TLS                     )

/* parameter for the CURLOPT_FTP_CREATE_MISSING_DIRS option */
FLG_ENTRY(FTP_CREATE_DIR_NONE             )
FLG_ENTRY(FTP_CREATE_DIR                  )
FLG_ENTRY(FTP_CREATE_DIR_RETRY            )
FLG_ENTRY(FTP_CREATE_DIR_LAST             )

/* parameter for the CURLOPT_FTP_FILEMETHOD option */
FLG_ENTRY(FTPMETHOD_DEFAULT               )
FLG_ENTRY(FTPMETHOD_MULTICWD              )
FLG_ENTRY(FTPMETHOD_NOCWD                 )
FLG_ENTRY(FTPMETHOD_SINGLECWD             )

/* bitmask defines for CURLOPT_HEADEROPT */
#if LCURL_CURL_VER_GE(7,37,0)
FLG_ENTRY(HEADER_UNIFIED                  )
FLG_ENTRY(HEADER_SEPARATE                 )
#endif

/* CURLPROTO_ defines are for the CURLOPT_*PROTOCOLS options */
FLG_ENTRY(PROTO_HTTP                      )
FLG_ENTRY(PROTO_HTTPS                     )
FLG_ENTRY(PROTO_FTP                       )
FLG_ENTRY(PROTO_FTPS                      )
FLG_ENTRY(PROTO_SCP                       )
FLG_ENTRY(PROTO_SFTP                      )
FLG_ENTRY(PROTO_TELNET                    )
FLG_ENTRY(PROTO_LDAP                      )
FLG_ENTRY(PROTO_LDAPS                     )
FLG_ENTRY(PROTO_DICT                      )
FLG_ENTRY(PROTO_FILE                      )
FLG_ENTRY(PROTO_TFTP                      )
#ifdef CURLPROTO_IMAP
FLG_ENTRY(PROTO_IMAP                      )
#endif
#ifdef CURLPROTO_IMAPS
FLG_ENTRY(PROTO_IMAPS                     )
#endif
#ifdef CURLPROTO_POP3
FLG_ENTRY(PROTO_POP3                      )
#endif
#ifdef CURLPROTO_POP3S
FLG_ENTRY(PROTO_POP3S                     )
#endif
#ifdef CURLPROTO_SMTP
FLG_ENTRY(PROTO_SMTP                      )
#endif
#ifdef CURLPROTO_SMTPS
FLG_ENTRY(PROTO_SMTPS                     )
#endif
#ifdef CURLPROTO_RTSP
FLG_ENTRY(PROTO_RTSP                      )
#endif
#ifdef CURLPROTO_RTMP
FLG_ENTRY(PROTO_RTMP                      )
#endif
#ifdef CURLPROTO_RTMPT
FLG_ENTRY(PROTO_RTMPT                     )
#endif
#ifdef CURLPROTO_RTMPE
FLG_ENTRY(PROTO_RTMPE                     )
#endif
#ifdef CURLPROTO_RTMPTE
FLG_ENTRY(PROTO_RTMPTE                    )
#endif
#ifdef CURLPROTO_RTMPS
FLG_ENTRY(PROTO_RTMPS                     )
#endif
#ifdef CURLPROTO_RTMPTS
FLG_ENTRY(PROTO_RTMPTS                    )
#endif
#ifdef CURLPROTO_GOPHER
FLG_ENTRY(PROTO_GOPHER                    )
#endif
#ifdef CURLPROTO_SMB
FLG_ENTRY(PROTO_SMB                       )
#endif
#ifdef CURLPROTO_SMBS
FLG_ENTRY(PROTO_SMBS                      )
#endif
#ifdef CURLPROTO_MQTT
FLG_ENTRY(PROTO_MQTT                      )
#endif
FLG_ENTRY(PROTO_ALL                       )

FLG_ENTRY(PROXY_HTTP                      ) /* added in 7.10.0 */
FLG_ENTRY(PROXY_HTTP_1_0                  ) /* added in 7.19.4 */
FLG_ENTRY(PROXY_SOCKS4                    ) /* added in 7.15.2 */
FLG_ENTRY(PROXY_SOCKS5                    ) /* added in 7.10.0 */
FLG_ENTRY(PROXY_SOCKS4A                   ) /* added in 7.18.0 */
FLG_ENTRY(PROXY_SOCKS5_HOSTNAME           ) /* added in 7.18.0 */
#if LCURL_CURL_VER_GE(7,52,0)
FLG_ENTRY(PROXY_HTTPS                     )
#endif

FLG_ENTRY(PAUSE_ALL                       ) /* added in 7.18.0 */
FLG_ENTRY(PAUSE_CONT                      ) /* added in 7.18.0 */
FLG_ENTRY(PAUSE_RECV                      ) /* added in 7.18.0 */
FLG_ENTRY(PAUSE_RECV_CONT                 ) /* added in 7.18.0 */
FLG_ENTRY(PAUSE_SEND                      ) /* added in 7.18.0 */
FLG_ENTRY(PAUSE_SEND_CONT                 ) /* added in 7.18.0 */

#if LCURL_CURL_VER_GE(7,64,1)
FLG_ENTRY(ALTSVC_H1)
FLG_ENTRY(ALTSVC_H2)
FLG_ENTRY(ALTSVC_H3)
FLG_ENTRY(ALTSVC_READONLYFILE)
#endif

#if LCURL_CURL_VER_GE(7,73,0)
FLG_ENTRY(PX_OK)
FLG_ENTRY(PX_BAD_ADDRESS_TYPE)
FLG_ENTRY(PX_BAD_VERSION)
FLG_ENTRY(PX_CLOSED)
FLG_ENTRY(PX_GSSAPI)
FLG_ENTRY(PX_GSSAPI_PERMSG)
FLG_ENTRY(PX_GSSAPI_PROTECTION)
FLG_ENTRY(PX_IDENTD)
FLG_ENTRY(PX_IDENTD_DIFFER)
FLG_ENTRY(PX_LONG_HOSTNAME)
FLG_ENTRY(PX_LONG_PASSWD)
FLG_ENTRY(PX_LONG_USER)
FLG_ENTRY(PX_NO_AUTH)
FLG_ENTRY(PX_RECV_ADDRESS)
FLG_ENTRY(PX_RECV_AUTH)
FLG_ENTRY(PX_RECV_CONNECT)
FLG_ENTRY(PX_RECV_REQACK)
FLG_ENTRY(PX_REPLY_ADDRESS_TYPE_NOT_SUPPORTED)
FLG_ENTRY(PX_REPLY_COMMAND_NOT_SUPPORTED)
FLG_ENTRY(PX_REPLY_CONNECTION_REFUSED)
FLG_ENTRY(PX_REPLY_GENERAL_SERVER_FAILURE)
FLG_ENTRY(PX_REPLY_HOST_UNREACHABLE)
FLG_ENTRY(PX_REPLY_NETWORK_UNREACHABLE)
FLG_ENTRY(PX_REPLY_NOT_ALLOWED)
FLG_ENTRY(PX_REPLY_TTL_EXPIRED)
FLG_ENTRY(PX_REPLY_UNASSIGNED)
FLG_ENTRY(PX_REQUEST_FAILED)
FLG_ENTRY(PX_RESOLVE_HOST)
FLG_ENTRY(PX_SEND_AUTH)
FLG_ENTRY(PX_SEND_CONNECT)
FLG_ENTRY(PX_SEND_REQUEST)
FLG_ENTRY(PX_UNKNOWN_FAIL)
FLG_ENTRY(PX_UNKNOWN_MODE)
FLG_ENTRY(PX_USER_REJECTED)
#endif

#if LCURL_CURL_VER_GE(7,73,0)
FLG_ENTRY(OT_LONG)
FLG_ENTRY(OT_VALUES)
FLG_ENTRY(OT_OFF_T)
FLG_ENTRY(OT_OBJECT)
FLG_ENTRY(OT_STRING)
FLG_ENTRY(OT_SLIST)
FLG_ENTRY(OT_CBPTR)
FLG_ENTRY(OT_BLOB)
FLG_ENTRY(OT_FUNCTION)
FLG_ENTRY(OT_FLAG_ALIAS)
#endif

#if LCURL_CURL_VER_GE(7,74,0)
FLG_ENTRY(HSTS_ENABLE)
FLG_ENTRY(HSTS_READONLYFILE)
FLG_ENTRY(STS_OK)
FLG_ENTRY(STS_DONE)
FLG_ENTRY(STS_FAIL)
#endif
