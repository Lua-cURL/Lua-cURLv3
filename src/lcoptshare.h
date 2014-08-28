#ifndef OPT_ENTRY
#  define OPT_ENTRY(a,b,c,d)
#endif

#ifndef FLG_ENTRY
#  define FLG_ENTRY(a)
#endif

OPT_ENTRY(share,                SHARE,                     LNG,     0 )
OPT_ENTRY(unshare,              UNSHARE,                   LNG,     0 )

FLG_ENTRY(LOCK_DATA_COOKIE)
FLG_ENTRY(LOCK_DATA_DNS)
FLG_ENTRY(LOCK_DATA_SSL_SESSION)

#undef OPT_ENTRY
#undef FLG_ENTRY