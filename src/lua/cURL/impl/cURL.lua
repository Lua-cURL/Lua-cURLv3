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

local module_info = {
  _NAME      = "Lua-cURL";
  _VERSION   = "0.3.13";
  _LICENSE   = "MIT";
  _COPYRIGHT = "Copyright (c) 2014-2022 Alexey Melnichuk";
}

local function Load_cURLv3(cURL, curl)

-------------------------------------------
local Form = common.class(curl.form, "LcURL Form") do

function Form:__init(opt)
  if opt then return self:add(opt) end
  return self
end

function Form:add(data)
  return common.form_add(self, data)
end

function Form:__tostring()
  local id = common.hash_id(tostring(self._handle))
  return string.format("%s %s (%s)", module_info._NAME, 'Form', id)
end

end
-------------------------------------------

-------------------------------------------
local Easy = common.class(curl.easy, "LcURL Easy") do

function Easy:__init(opt)
  if opt then return self:setopt(opt) end
  return self
end

local perform = common.wrap_function("perform")
function Easy:perform(opt)
  if opt then
    local ok, err = self:setopt(opt)
    if not ok then return nil, err end
  end

  return perform(self)
end

local setopt_httppost = common.wrap_function("setopt_httppost")
function Easy:setopt_httppost(form)
  return setopt_httppost(self, form:handle())
end

if curl.OPT_STREAM_DEPENDS then

local setopt_stream_depends = common.wrap_function("setopt_stream_depends")
function Easy:setopt_stream_depends(easy)
  return setopt_stream_depends(self, easy:handle())
end

local setopt_stream_depends_e = common.wrap_function("setopt_stream_depends_e")
function Easy:setopt_stream_depends_e(easy)
  return setopt_stream_depends_e(self, easy:handle())
end

end

local setopt = common.wrap_function("setopt")
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
      t = t2 or common.clone(t); t2 = t;
      if t.httppost           then t.httppost           = hpost:handle() end
      if t[curl.OPT_HTTPPOST] then t[curl.OPT_HTTPPOST] = hpost:handle() end
    end

    local easy = t.stream_depends or t[curl.OPT_STREAM_DEPENDS]
    if easy and easy._handle then
      t = t2 or common.clone(t); t2 = t;
      if t.stream_depends           then t.stream_depends           = easy:handle() end
      if t[curl.OPT_STREAM_DEPENDS] then t[curl.OPT_STREAM_DEPENDS] = easy:handle() end
    end

    local easy = t.stream_depends_e or t[curl.OPT_STREAM_DEPENDS_E]
    if easy and easy._handle then
      t = t2 or common.clone(t); t2 = t;
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
  local id = common.hash_id(tostring(self._handle))
  return string.format("%s %s (%s)", module_info._NAME, 'Easy', id)
end

end
-------------------------------------------

-------------------------------------------
local Multi = common.class(curl.multi, "LcURL Multi") do

local add_handle    = common.wrap_function("add_handle")
local remove_handle = common.wrap_function("remove_handle")

function Multi:__init(opt)
  self._easy = {n = 0}
  if opt then self:setopt(opt) end
  return self
end

function Multi:iperform()
  return common.make_iterator(self, self.perform)
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

local setopt_socketfunction = common.wrap_function("setopt_socketfunction")
function Multi:setopt_socketfunction(...)
  local cb = wrap_callback(...)

  return setopt_socketfunction(self, wrap_socketfunction(self, cb))
end

local setopt = common.wrap_function("setopt")
function Multi:setopt(k, v)
  if type(k) == 'table' then
    local t = k

    local socketfunction = t.socketfunction or t[curl.OPT_SOCKETFUNCTION]
    if socketfunction then
      t = common.clone(t)
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
  local id = common.hash_id(tostring(self._handle))
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
  local cURL = common.clone(module_info)

  Load_cURLv3(cURL, curl)

  local ok, Load_cURLv2 = pcall(require, "cURL.impl.cURLv2")
  if ok then
    Load_cURLv2(cURL, curl)
  end

  return cURL
end