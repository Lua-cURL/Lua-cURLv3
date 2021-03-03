cmake_minimum_required (VERSION 2.6)
project (lcurl)

#luajit's usage
#gcc -I/opt/openresty/luajit/include/luajit-2.1 -L/opt/openresty/luajit/lib -lluajit-5.1 -O2 -g -Wall -fPIC --shared usertime.c -o usertime.so
INCLUDE_DIRECTORIES(/opt/openresty/luajit/include/luajit-2.1 )
LINK_DIRECTORIES(/opt/openresty/luajit/lib)
LINK_LIBRARIES(curl)

file(GLOB lcurl_sources ./src/**.c)
SET(lcurl ${lcurl_sources})

ADD_LIBRARY (lcurl SHARED ${lcurl})
#output to lcurl.so not the default liblcurl.so
SET_TARGET_PROPERTIES(lcurl PROPERTIES PREFIX "")

#If deploy to resty-luajit, you can add below
MESSAGE("Copy to Openresty: ${PROJECT_BINARY_DIR}")
execute_process(
# lib so
 COMMAND cp -f "${PROJECT_BINARY_DIR}/lcurl.so" /opt/openresty/lualib/.
#lib lua 
 COMMAND cp -f "${PROJECT_SOURCE_DIR}/src/lua/cURL.lua" /opt/openresty/lualib/
 COMMAND cp -rf "${PROJECT_SOURCE_DIR}/src/lua/cURL" /opt/openresty/lualib/
 WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
