--
--  Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Copyright (C) 2014-2021 Alexey Melnichuk <alexeymelnichuck@gmail.com>
--
--  Licensed according to the included 'LICENSE' document
--
--  This file is part of Lua-cURL library.
--

local module_info = {
  _NAME      = "Lua-cURL";
  _VERSION   = "0.3.13";
  _LICENSE   = "MIT";
  _COPYRIGHT = "Copyright (c) 2014-2021 Alexey Melnichuk";
}

local function hash_id(str)
  local id = string.match(str, "%((.-)%)") or string.match(str, ': (%x+)$')
  return id
end

local function clone(t, o)
  o = o or {}
  for k,v in pairs(t) do o[k]=v end
  return o
end

local function wrap_function(k)
  return function(self, ...)
    local ok, err = self._handle[k](self._handle, ...)
    if ok == self._handle then return self end
    return ok, err
  end
end

local function wrap_setopt_flags(k, flags)
  k = "setopt_" .. k
  local flags2 = clone(flags)
  for k, v in pairs(flags) do flags2[v] = v end

  return function(self, v)
    v = assert(flags2[v], "Unsupported value " .. tostring(v))
    local ok, err = self._handle[k](self._handle, v)
    if ok == self._handle then return self end
    return ok, err
  end
end

local function new_buffers()
  local buffers = {resp = {}, _ = {}}

  function buffers:append(e, ...)
    local resp = assert(e:getinfo_response_code())
    if not self._[e] then self._[e] = {} end

    local b = self._[e]

    if self.resp[e] ~= resp then
      b[#b + 1] = {"response", resp}
      self.resp[e] = resp
    end

    b[#b + 1] = {...}
  end

  function buffers:next()
    for e, t in pairs(self._) do
      local m = table.remove(t, 1)
      if m then return e, m end
    end
  end

  return buffers
end

local function make_iterator(self, perform)
  local curl = require "lcurl.safe"

  local buffers = new_buffers()

  -- reset callbacks to all easy handles
  local function reset_easy(self)
    if not self._easy_mark then -- that means we have some new easy handles
      for h, e in pairs(self._easy) do if h ~= 'n' then 
          e:setopt_writefunction (function(str) buffers:append(e, "data",   str) end)
          e:setopt_headerfunction(function(str) buffers:append(e, "header", str) end)
      end end
      self._easy_mark = true
    end
    return self._easy.n
  end

  if 0 == reset_easy(self) then return end

  assert(perform(self))

  return function()
    -- we can add new handle during iteration
    local remain = reset_easy(self)

    -- wait next event
    while true do
      local e, t = buffers:next()
      if t then return t[2], t[1], e end
      if remain == 0 then break end

      self:wait()

      local n = assert(perform(self))

      if n <= remain then
        while true do
          local e, ok, err = assert(self:info_read())
          if e == 0 then break end
          if ok then
            ok = e:getinfo_response_code() or ok
            buffers:append(e, "done", ok)
          else buffers:append(e, "error", err) end
          self:remove_handle(e)
          e:unsetopt_headerfunction()
          e:unsetopt_writefunction()
        end
      end

      remain = n
    end
  end
end

-- name = <string>/<stream>/<file>/<buffer>/<content>
--
-- <stream> = {
--   stream  = function/object
--   length  = ?number
--   name    = ?string
--   type    = ?string
--   headers = ?table
-- }
--
-- <file> = {
--   file    = string
--   type    = ?string
--   name    = ?string
--   headers = ?table
-- }
--
-- <buffer> = {
--   data    = string
--   name    = string
--   type    = ?string
--   headers = ?table
-- }
--
-- <content> = {
--   content = string -- or first key in table
--   type    = ?string
--   headers = ?table
-- }
-- 

local function form_add_element(form, name, value)
  local vt = type(value)
  if vt == "string" then return form:add_content(name, value) end

  assert(type(name) == "string")
  assert(vt == "table")
  assert((value.name    == nil) or (type(value.name   ) == 'string'))
  assert((value.type    == nil) or (type(value.type   ) == 'string'))
  assert((value.headers == nil) or (type(value.headers) == 'table' ))

  if value.stream then
    local vst = type(value.stream)

    if vst == 'function' then
      assert(type(value.length) == 'number')
      local length = value.length
      return form:add_stream(name, value.name, value.type, value.headers, length, value.stream)
    end

    if (vst == 'table') or (vst == 'userdata') then
      local length = value.length or assert(value.stream:length())
      assert(type(length) == 'number')
      return form:add_stream(name, value.name, value.type, value.headers, length, value.stream)
    end

    error("Unsupported stream type: " .. vst)
  end

  if value.file then
    assert(type(value.file) == 'string')
    return form:add_file(name, value.file, value.type, value.filename, value.headers)
  end

  if value.data then
    assert(type(value.data) == 'string')
    assert(type(value.name) == 'string')
    return form:add_buffer(name, value.name, value.data, value.type, value.headers)
  end

  local content = value[1] or value.content
  if content then
    assert(type(content) == 'string')
    if value.type then
      return form:add_content(name, content, value.type, value.headers)
    end
    return form:add_content(name, content, value.headers)
  end

  return form
end

local function form_add(form, data)
  for k, v in pairs(data) do
    local ok, err = form_add_element(form, k, v)
    if not ok then return nil, err end
  end

  return form
end

local function class(ctor)
  local C = {}
  C.__index = function(self, k)
    local fn = C[k]

    if not fn and self._handle[k] then
      fn = wrap_function(k)
      C[k] = fn
    end
    return fn
  end

  function C:new(...)
    local h, err = ctor()
    if not h then return nil, err end

    local o = setmetatable({
      _handle = h
    }, self)

    if self.__init then return self.__init(o, ...) end

    return o
  end

  function C:handle()
    return self._handle
  end

  return C
end

local function Load_cURLv2(cURL, curl)

-------------------------------------------
local Easy = class(curl.easy) do

local perform             = wrap_function("perform")
local setopt_share        = wrap_function("setopt_share")
local setopt_readfunction = wrap_function("setopt_readfunction")

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
local Multi = class(curl.multi) do

local perform       = wrap_function("perform")
local add_handle    = wrap_function("add_handle")
local remove_handle = wrap_function("remove_handle")

function Multi:__init()
  self._easy = {n = 0}
  return self
end

function Multi:perform()
  return make_iterator(self, perform)
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
local Share = class(curl.share) do

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

local function Load_cURLv3(cURL, curl)

-------------------------------------------
local Form = class(curl.form) do

function Form:__init(opt)
  if opt then return self:add(opt) end
  return self
end

function Form:add(data)
  return form_add(self, data)
end

function Form:__tostring()
  local id = hash_id(tostring(self._handle))
  return string.format("%s %s (%s)", module_info._NAME, 'Form', id)
end

end
-------------------------------------------

-------------------------------------------
local Easy = class(curl.easy) do

function Easy:__init(opt)
  if opt then return self:setopt(opt) end
  return self
end

local perform = wrap_function("perform")
function Easy:perform(opt)
  if opt then
    local ok, err = self:setopt(opt)
    if not ok then return nil, err end
  end

  return perform(self)
end

local setopt_httppost = wrap_function("setopt_httppost")
function Easy:setopt_httppost(form)
  return setopt_httppost(self, form:handle())
end

if curl.OPT_STREAM_DEPENDS then

local setopt_stream_depends = wrap_function("setopt_stream_depends")
function Easy:setopt_stream_depends(easy)
  return setopt_stream_depends(self, easy:handle())
end

local setopt_stream_depends_e = wrap_function("setopt_stream_depends_e")
function Easy:setopt_stream_depends_e(easy)
  return setopt_stream_depends_e(self, easy:handle())
end

end

local setopt = wrap_function("setopt")
local custom_setopt = {
  [curl.OPT_HTTPPOST         or true] = 'setopt_httppost';
  [curl.OPT_STREAM_DEPENDS   or true] = 'setopt_stream_depends';
  [curl.OPT_STREAM_DEPENDS_E or true] = 'setopt_stream_depends_e';
}
custom_setopt[true] = nil

function Easy:setopt(k, v)
  if type(k) == 'table' then
    local t = k

    local t2
    local hpost = t.httppost or t[curl.OPT_HTTPPOST]
    if hpost and hpost._handle then
      t = t2 or clone(t); t2 = t;
      if t.httppost           then t.httppost           = hpost:handle() end
      if t[curl.OPT_HTTPPOST] then t[curl.OPT_HTTPPOST] = hpost:handle() end
    end

    local easy = t.stream_depends or t[curl.OPT_STREAM_DEPENDS]
    if easy and easy._handle then
      t = t2 or clone(t); t2 = t;
      if t.stream_depends           then t.stream_depends           = easy:handle() end
      if t[curl.OPT_STREAM_DEPENDS] then t[curl.OPT_STREAM_DEPENDS] = easy:handle() end
    end

    local easy = t.stream_depends_e or t[curl.OPT_STREAM_DEPENDS_E]
    if easy and easy._handle then
      t = t2 or clone(t); t2 = t;
      if t.stream_depends_e           then t.stream_depends_e           = easy:handle() end
      if t[curl.OPT_STREAM_DEPENDS_E] then t[curl.OPT_STREAM_DEPENDS_E] = easy:handle() end
    end

    return setopt(self, t)
  end

  local setname = custom_setopt[k]
  if setname then
    return self[setname](self, v)
  end

  return setopt(self, k, v)
end

function Easy:__tostring()
  local id = hash_id(tostring(self._handle))
  return string.format("%s %s (%s)", module_info._NAME, 'Easy', id)
end

end
-------------------------------------------

-------------------------------------------
local Multi = class(curl.multi) do

local add_handle    = wrap_function("add_handle")
local remove_handle = wrap_function("remove_handle")

function Multi:__init(opt)
  self._easy = {n = 0}
  if opt then self:setopt(opt) end
  return self
end

function Multi:iperform()
  return make_iterator(self, self.perform)
end

function Multi:add_handle(e)
  assert(self._easy.n >= 0)

  local h = e:handle()
  if self._easy[h] then
    return nil, curl.error(curl.ERROR_MULTI, curl.E_MULTI_ADDED_ALREADY or curl.E_MULTI_BAD_EASY_HANDLE)
  end

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

local function wrap_callback(...)
  local n = select("#", ...)
  local fn, ctx, has_ctx
  if n >= 2 then
    has_ctx, fn, ctx = true, assert(...)
  else
    fn = assert(...)
    if type(fn) ~= "function" then
      has_ctx, fn, ctx = true, assert(fn.socket), fn
    end
  end
  if has_ctx then
    return function(...) return fn(ctx, ...) end
  end
  return function(...) return fn(...) end
end

local function wrap_socketfunction(self, cb)
  local ptr = setmetatable({value = self},{__mode = 'v'})
  return function(h, ...)
    local e = ptr.value._easy[h]
    if e then return cb(e, ...) end
    return 0
  end
end

local setopt_socketfunction = wrap_function("setopt_socketfunction")
function Multi:setopt_socketfunction(...)
  local cb = wrap_callback(...)

  return setopt_socketfunction(self, wrap_socketfunction(self, cb))
end

local setopt = wrap_function("setopt")
function Multi:setopt(k, v)
  if type(k) == 'table' then
    local t = k

    local socketfunction = t.socketfunction or t[curl.OPT_SOCKETFUNCTION]
    if socketfunction then
      t = clone(t)
      local fn = wrap_socketfunction(self, socketfunction)
      if t.socketfunction           then t.socketfunction           = fn end
      if t[curl.OPT_SOCKETFUNCTION] then t[curl.OPT_SOCKETFUNCTION] = fn end
    end

    return setopt(self, t)
  end

  if k == curl.OPT_SOCKETFUNCTION then
    return self:setopt_socketfunction(v)
  end

  return setopt(self, k, v)
end

function Multi:__tostring()
  local id = hash_id(tostring(self._handle))
  return string.format("%s %s (%s)", module_info._NAME, 'Multi', id)
end

end
-------------------------------------------

setmetatable(cURL, {__index = curl})

function cURL.form(...)  return Form:new(...)  end

function cURL.easy(...)  return Easy:new(...)  end

function cURL.multi(...) return Multi:new(...) end

end

return function(curl)
  local cURL = clone(module_info)

  Load_cURLv3(cURL, curl)

  Load_cURLv2(cURL, curl)

  return cURL
end
