#!/usr/bin/env lua

package = 'lua-http-parser'
version = '2.7-1'
source = {
    url = 'gitrec+https://github.com/brimworks/lua-http-parser'
}
description = {
    summary  = "A Lua binding to Ryan Dahl's http request/response parser.",
    detailed = '',
    homepage = 'http://github.com/brimworks/lua-http-parser',
    license  = 'MIT', --as with Ryan's
}
dependencies = {
    'lua >= 5.1, < 5.5',
    'luarocks-fetch-gitrec',
}
build = {
    type = 'builtin',
    modules = {
        ['http.parser'] = {
            sources = {
                "http-parser/http_parser.c",
                "lua-http-parser.c"
            }
        }
    }
}
