T=lcurl

#default installtion prefix
PREFIX=/usr/local

LUA_VERSION = $(shell pkg-config luajit --print-provides)

ifeq ($(LUA_VERSION),)
LUA_CFLAGS=$(shell pkg-config lua --cflags)
LUA_LIBS=$(shell pkg-config lua --libs)
else
LUA_CFLAGS=$(shell pkg-config luajit --cflags)
LUA_LIBS=$(shell pkg-config luajit --libs)
endif


CURL_CFLAGS=$(shell pkg-config libcurl --cflags)
CURL_LIBS=$(shell pkg-config libcurl --libs)

# System's libraries directory (where binary libraries are installed)
LUA_LIBDIR= $(PREFIX)/lib/lua/5.1

# OS dependent
LIB_OPTION= -shared #for Linux

#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X
LIBNAME= $T.so.$V

# Compilation directives
WARN_MOST= -Wall -fPIC -W -Waggregate-return -Wcast-align -Wmissing-prototypes -Wnested-externs -Wshadow -Wwrite-strings -pedantic
WARN= -Wall -fPIC -Wno-unused-value
CFLAGS= $(WARN) -DPTHREADS $(LUA_CFLAGS) $(CURL_CFLAGS)
CC= gcc -g -shared -fPIC $(CFLAGS)
LIB_OPTION=$(LUA_LIBS) $(CURL_LIBS)

SRCS=$(shell echo src/*.c)

all: $T.so

$T.so: $(SRCS)
	MACOSX_DEPLOYMENT_TARGET="10.3"; export MACOSX_DEPLOYMENT_TARGET; $(CC) $(CFLAGS) $(LIB_OPTION) -o $T.so $(SRCS) -lrt -ldl

install: all
	mkdir -p $(LUA_LIBDIR)
	cp $T.so $(LUA_LIBDIR)

clean:
	rm -f $T.so $(OBJS) 


