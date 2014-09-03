T=lcurl

#default installtion prefix
PREFIX=/usr/local

UNAME=$(shell uname)

ifeq ($(UNAME),Linux)		###########################Linux
#try to use luajit     
LUA_VERSION = $(shell pkg-config luajit --print-provides)
ifeq ($(LUA_VERSION),)		###########################Lua
LUA_CFLAGS=-I$(PREFIX)/include
LUA_LIBS=-L$(PREFIX)/lib 
else                   		###########################Luajit
LUA_CFLAGS=$(shell pkg-config luajit --cflags)
LUA_LIBS=$(shell pkg-config luajit --libs)
endif

CURL_CFLAGS=$(shell pkg-config libcurl --cflags)
CURL_LIBS=$(shell pkg-config libcurl --libs)

CC= gcc -g -fPIC -shared
LIB_OPTION= -lrt -ldl

else			  	#####other platform

LUA_CFLAGS=-I$(PREFIX)/include
LUA_LIBS=-llua5.1
CURL_CFLAGS=-I$(PREFIX)/include
CURL_LIBS=-L$(PREFIX)/lib -lcurl

CC= gcc -g -shared  $(CFLAGS)

endif



# System's libraries directory (where binary libraries are installed)
LUA_LIBDIR= $(PREFIX)/lib/lua/5.1

# OS dependent

#LIB_OPTION= -bundle -undefined dynamic_lookup #for MacOS X
LIBNAME= $T.so.$V

# Compilation directives
WARN_MOST= -Wall -W -Waggregate-return -Wcast-align -Wmissing-prototypes -Wnested-externs -Wshadow -Wwrite-strings -pedantic
WARN= -Wall -Wno-unused-value
CFLAGS= $(WARN) -DPTHREADS $(LUA_CFLAGS) $(CURL_CFLAGS) 
CC+= $(CFLAGS)
SRCS=$(shell echo src/*.c)

all: $T.so

$T.so: $(SRCS)
	MACOSX_DEPLOYMENT_TARGET="10.3"; export MACOSX_DEPLOYMENT_TARGET; $(CC) $(CFLAGS)  -o $T.so $(SRCS)  $(CURL_LIBS) $(LUA_LIBS) $(LIB_OPTION)

install: all
	mkdir -p $(LUA_LIBDIR)
	cp $T.so $(LUA_LIBDIR)

clean:
	rm -f $T.so $(OBJS) 


