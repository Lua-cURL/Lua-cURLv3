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
  "lua >= 5.1, < 5.3"
}

external_dependencies = {
  CURL = {
    header = "curl/curl.h"
  }
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
    cURL  = "src/lua/cURL.lua",
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