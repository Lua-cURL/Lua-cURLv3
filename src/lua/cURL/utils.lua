--
--  Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Licensed according to the included 'LICENSE' document
--
--  This file is part of Lua-cURL library.
--

--- Returns path to cURL ca bundle
--
-- @tparam[opt="curl-ca-bundle.crt"] string name name of bundle
-- @treturn string path to file (CURLOPT_CAINFO)
-- @treturn string path to ssl dir path (CURLOPT_CAPATH)
--
-- @usage 
--  local file, path = find_ca_bundle()
--  if file then e:setopt_cainfo(file) end
--  if path then e:setopt_capath(path) end
--
local function find_ca_bundle(name)
  name = name or "curl-ca-bundle.crt"

  local path  = require "path"
  local env   = setmetatable({},{__index = function(_, name) return os.getenv(name) end})

  local function split(str, sep, plain)
    local b, res = 1, {}
    while b <= #str do
      local e, e2 = string.find(str, sep, b, plain)
      if e then
        table.insert(res, (string.sub(str, b, e-1)))
        b = e2 + 1
      else
        table.insert(res, (string.sub(str, b)))
        break
      end
    end
    return res
  end

  if env.CURL_CA_BUNDLE and path.isfile(env.CURL_CA_BUNDLE) then
    return env.CURL_CA_BUNDLE
  end

  if env.SSL_CERT_DIR and path.isdir(env.SSL_CERT_DIR) then
    return nil, env.SSL_CERT_DIR
  end

  if env.SSL_CERT_FILE and path.isfile(env.SSL_CERT_FILE) then
    return env.SSL_CERT_FILE
  end

  if not path.IS_WINDOWS then return end

  local paths = {
    '.',
    path.join(env.windir, "System32"),
    path.join(env.windir, "SysWOW64"),
    env.windir,
  }
  for _, p in ipairs(split(env.path, ';')) do paths[#paths + 1] = p end

  for _, p in ipairs(paths) do
    p = path.fullpath(p)
    if path.isdir(p) then
      p = path.join(p, name)
      if path.isfile(p) then
        return p
      end
    end
  end
end

return {
  find_ca_bundle = find_ca_bundle;
}

