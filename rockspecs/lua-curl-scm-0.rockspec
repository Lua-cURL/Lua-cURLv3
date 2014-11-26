package = "Lua-cURL"
version = "scm-0"

source = {
  url = "https://github.com/Lua-cURL/Lua-cURLv3/archive/master.zip",
  dir = "Lua-cURLv3-master",
}

description = {
  summary = "Lua binding to libcurl",
  detailed = [[
  ]],
  homepage = "https://github.com/Lua-cURL",
  license  = "MIT/X11"
}

dependencies = {
  "lua >= 5.1, < 5.4"
}

external_dependencies = {
  platforms = {
    windows = {
      CURL = {
        header  = "curl/curl.h",
        library = "libcurl",
      }
    };
    unix = {
      CURL = {
        header  = "curl/curl.h",
        -- library = "curl",
      }
    };
  }
}

build = {
  copy_directories = {'doc', 'examples'},

  type = "builtin",

  platforms = {
    windows = { modules = {
      lcurl = {
        libraries = {"libcurl", "ws2_32"},
      }
    }},
    unix    = { modules = {
      lcurl = {
        libraries = {"curl"},
      }
    }}
  },

  modules = {
    ["cURL"           ] = "src/lua/cURL.lua",
    ["cURL.safe"      ] = "src/lua/cURL/safe.lua",
    ["cURL.utils"     ] = "src/lua/cURL/utils.lua",
    ["cURL.impl.cURL" ] = "src/lua/cURL/impl/cURL.lua",

    lcurl = {
      sources = {
        "src/l52util.c",    "src/lceasy.c", "src/lcerror.c",
        "src/lchttppost.c", "src/lcurl.c",  "src/lcutils.c",
        "src/lcmulti.c",    "src/lcshare.c",
      },
      incdirs   = { "$(CURL_INCDIR)" },
      libdirs   = { "$(CURL_LIBDIR)" }
    },
  }
}


