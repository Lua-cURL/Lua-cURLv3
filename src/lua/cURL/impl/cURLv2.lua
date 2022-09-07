--
--  Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Copyright (C) 2014-2022 Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Licensed according to the included 'LICENSE' document
--
--  This file is part of Lua-cURL library.
--

local common = require "cURL.impl.common"

local function wrap_setopt_flags(k, flags)
  k = "setopt_" .. k
  local flags2 = common.clone(flags)
  for k, v in pairs(flags) do flags2[v] = v end

  return function(self, v)
    v = assert(flags2[v], "Unsupported value " .. tostring(v))
    local ok, err = self._handle[k](self._handle, v)
    if ok == self._handle then return self end
    return ok, err
  end
end

local function Load_cURLv2(cURL, curl)

-------------------------------------------
local Easy = common.class(curl.easy, "LcURL Easy") do

local perform             = common.wrap_function("perform")
local setopt_share        = common.wrap_function("setopt_share")
local setopt_readfunction = common.wrap_function("setopt_readfunction")

local NONE = {}

function Easy:_call_readfunction(...)
  if self._rd_ud == NONE then
    return self._rd_fn(...)
  end
  return self._rd_fn(self._rd_ud, ...)
end

function Easy:setopt_readfunction(fn, ...)
  assert(fn)

  if select('#', ...) == 0 then
    if type(fn) == "function" then
      self._rd_fn = fn
      self._rd_ud = NONE
    else
      self._rd_fn = assert(fn.read)
      self._rd_ud = fn
    end
  else
    self._rd_fn = fn
    self._ud_fn = ...
  end

  return setopt_readfunction(self, fn, ...)
end

function Easy:perform(opt)
  opt = opt or {}

  local oerror = opt.errorfunction or function(err) return nil, err end

  if opt.readfunction then
    local ok, err = self:setopt_readfunction(opt.readfunction)
    if not ok then return oerror(err) end
  end

  if opt.writefunction then
    local ok, err = self:setopt_writefunction(opt.writefunction)
    if not ok then return oerror(err) end
  end

  if opt.headerfunction then
    local ok, err = self:setopt_headerfunction(opt.headerfunction)
    if not ok then return oerror(err) end
  end

  local ok, err = perform(self)
  if not ok then return oerror(err) end

  return self 
end

function Easy:post(data)
  local form = curl.form()
  local ok, err = true

  for k, v in pairs(data) do
    if type(v) == "string" then
      ok, err = form:add_content(k, v)
    else
      assert(type(v) == "table")
      if v.stream_length then
        local len = assert(tonumber(v.stream_length))
        assert(v.file)
        if v.stream then
          ok, err = form:add_stream(k, v.file, v.type, v.headers, len, v.stream)
        else
          ok, err = form:add_stream(k, v.file, v.type, v.headers, len, self._call_readfunction, self)
        end
      elseif v.data then
        ok, err = form:add_buffer(k, v.file, v.data, v.type, v.headers)
      else
        ok, err = form:add_file(k, v.file, v.type, v.filename, v.headers)
      end
    end
    if not ok then break end
  end

  if not ok then
    form:free()
    return nil, err
  end

  ok, err = self:setopt_httppost(form)
  if not ok then
    form:free()
    return nil, err
  end

  return self
end

function Easy:setopt_share(s)
  return setopt_share(self, s:handle())
end

Easy.setopt_proxytype = wrap_setopt_flags("proxytype", {
  ["HTTP"            ] = curl.PROXY_HTTP;
  ["HTTP_1_0"        ] = curl.PROXY_HTTP_1_0;
  ["SOCKS4"          ] = curl.PROXY_SOCKS4;
  ["SOCKS5"          ] = curl.PROXY_SOCKS5;
  ["SOCKS4A"         ] = curl.PROXY_SOCKS4A;
  ["SOCKS5_HOSTNAME" ] = curl.PROXY_SOCKS5_HOSTNAME;
  ["HTTPS"           ] = curl.PROXY_HTTPS;
})

Easy.setopt_httpauth  = wrap_setopt_flags("httpauth", {
  ["NONE"            ] = curl.AUTH_NONE;
  ["BASIC"           ] = curl.AUTH_BASIC;
  ["DIGEST"          ] = curl.AUTH_DIGEST;
  ["GSSNEGOTIATE"    ] = curl.AUTH_GSSNEGOTIATE;
  ["NEGOTIATE"       ] = curl.AUTH_NEGOTIATE;
  ["NTLM"            ] = curl.AUTH_NTLM;
  ["DIGEST_IE"       ] = curl.AUTH_DIGEST_IE;
  ["GSSAPI"          ] = curl.AUTH_GSSAPI;
  ["NTLM_WB"         ] = curl.AUTH_NTLM_WB;
  ["ONLY"            ] = curl.AUTH_ONLY;
  ["ANY"             ] = curl.AUTH_ANY;
  ["ANYSAFE"         ] = curl.AUTH_ANYSAFE;
  ["BEARER"          ] = curl.AUTH_BEARER;
})

Easy.setopt_ssh_auth_types = wrap_setopt_flags("ssh_auth_types", {
  ["NONE"        ] = curl.SSH_AUTH_NONE;
  ["ANY"         ] = curl.SSH_AUTH_ANY;
  ["PUBLICKEY"   ] = curl.SSH_AUTH_PUBLICKEY;
  ["PASSWORD"    ] = curl.SSH_AUTH_PASSWORD;
  ["HOST"        ] = curl.SSH_AUTH_HOST;
  ["GSSAPI"      ] = curl.SSH_AUTH_GSSAPI;
  ["KEYBOARD"    ] = curl.SSH_AUTH_KEYBOARD;
  ["AGENT"       ] = curl.SSH_AUTH_AGENT;
  ["DEFAULT"     ] = curl.SSH_AUTH_DEFAULT;
})

end
-------------------------------------------

-------------------------------------------
local Multi = common.class(curl.multi, "LcURL Multi") do

local perform       = common.wrap_function("perform")
local add_handle    = common.wrap_function("add_handle")
local remove_handle = common.wrap_function("remove_handle")

function Multi:__init()
  self._easy = {n = 0}
  return self
end

function Multi:perform()
  return common.make_iterator(self, perform)
end

function Multi:add_handle(e)
  assert(self._easy.n >= 0)

  local h = e:handle()
  if self._easy[h] then return self end

  local ok, err = add_handle(self, h)
  if not ok then return nil, err end
  self._easy[h], self._easy.n = e, self._easy.n + 1
  self._easy_mark = nil

  return self
end

function Multi:remove_handle(e)
  local h = e:handle()

  if self._easy[h] then
    self._easy[h], self._easy.n = nil, self._easy.n - 1
  end
  assert(self._easy.n >= 0)

  return remove_handle(self, h)
end

function Multi:info_read(...)
  while true do
    local h, ok, err = self:handle():info_read(...)
    if not h then return nil, ok end
    if h == 0 then return h end

    local e = self._easy[h]
    if e then
      if ... then
        self._easy[h], self._easy.n = nil, self._easy.n - 1
      end
      return e, ok, err
    end
  end
end

end
-------------------------------------------

-------------------------------------------
local Share = common.class(curl.share, "LcURL Share") do

Share.setopt_share = wrap_setopt_flags("share", {
  [ "COOKIE"      ] = curl.LOCK_DATA_COOKIE;
  [ "DNS"         ] = curl.LOCK_DATA_DNS;
  [ "SSL_SESSION" ] = curl.LOCK_DATA_SSL_SESSION;
})

end
-------------------------------------------

assert(cURL.easy_init == nil)
function cURL.easy_init()  return Easy:new()  end

assert(cURL.multi_init == nil)
function cURL.multi_init() return Multi:new() end

assert(cURL.share_init == nil)
function cURL.share_init() return Share:new() end

end

return Load_cURLv2