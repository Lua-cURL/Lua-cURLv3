#ifndef OPT_ENTRY
#  define OPT_ENTRY(a,b,c,d)
#  define OPT_ENTRY_IS_NULL
#endif

#ifndef FLG_ENTRY
#  define FLG_ENTRY(a)
#  define FLG_ENTRY_IS_NULL
#endif

OPT_ENTRY(share,                SHARE,                     LNG,     0 )
OPT_ENTRY(unshare,              UNSHARE,                   LNG,     0 )

FLG_ENTRY( LOCK_DATA_COOKIE                  )
FLG_ENTRY( LOCK_DATA_DNS                     )
FLG_ENTRY( LOCK_DATA_SSL_SESSION             )
FLG_ENTRY( LOCK_DATA_CONNECT                 )

#ifdef OPT_ENTRY_IS_NULL
#  undef OPT_ENTRY
#  undef OPT_ENTRY_IS_NULL
#endif

#ifdef FLG_ENTRY_IS_NULL
#  undef FLG_ENTRY
#  undef FLG_ENTRY_IS_NULL
#endif
