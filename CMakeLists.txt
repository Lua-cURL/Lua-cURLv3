cmake_minimum_required (VERSION 2.6)
project (lcurl)

file(GLOB lcurl_sources ./src/**.c)
SET(lcurl ${lcurl_sources})

ADD_LIBRARY (lcurl  ${lcurl})
TARGET_LINK_LIBRARIES (lcurl)