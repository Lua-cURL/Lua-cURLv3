package = "lcurl"
version = "scm-0"
source = {
  url = "https://github.com/moteus/lua-lcurl/archive/master.zip",
  dir = "lua-lcurl-master",
}
description = {
  summary = "Lua binding to libcurl",
  detailed = [[
  ]],
  homepage = "https://github.com/moteus/lua-lcurl",
  license = "MIT/X11"
}
dependencies = {
  "lua >= 5.1"
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
        libraries = {"libcurl"},
      }
    }},
    unix    = { modules = {
      lcurl = {
        libraries = {"curl"},
      }
    }}
  },

  modules = {
    lcurl = {
      sources = {
        "src/l52util.c",    "src/lceasy.c", "src/lcerror.c",
        "src/lchttppost.c", "src/lcurl.c",  "src/lcutils.c"
      },
      incdirs   = { "$(CURL_INCDIR)" },
      libdirs   = { "$(CURL_LIBDIR)" }
    }
  }
}