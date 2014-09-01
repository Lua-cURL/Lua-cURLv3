--- High level wrapper around lcurl.easy
--
-- Usage:
--
-- local e = cURL.easy{
--   url            = 'http://example.com',
--   writefunction  = io.stdout;
--   headerfunction = function(msg) io.stderr:write(msg) end;
-- }
-- e:perform()
--

local curl = require "lcurl.safe"

local easy = {} do

local LCURL_ERROR_EASY = curl.ERROR_EASY
local CURLE_UNKNOWN_OPTION = curl.E_UNKNOWN_OPTION

easy.__index = function(self, k)
  if k:sub(1, 4) == 'set_' then
    local h = self:handle()
    local name = k:sub(5)
    if h['setopt_' .. name] then 
      easy[k] = function(self, value) return self:set(name, value) end
    end
  elseif k:sub(1, 4) == 'get_' then
    local h = self:handle()
    local name = k:sub(5)
    if h['getinfo_' .. name] then 
      easy[k] = function(self) return self:get(name) end
    end
  end
  return easy[k]
end

local function setopt(handle, k, v, ...)
  local fn = handle["setopt_" .. k]
  if not fn then
    return nil, curl.error(LCURL_ERROR_EASY,  CURLE_UNKNOWN_OPTION)
  end
  local ok, err = fn(handle, v, ...)
  if not ok then return nil, err end
  return true
end

function easy:new(opt)
  local handle, err = curl.easy()
  if not handle then return nil, err end
 
  if opt then for k, v in pairs(opt) do
    local ok, err = setopt(handle, k, v)
    if not ok then
      handle:close()
      return nil, err
    end
  end end
 
  local o = setmetatable({
    _handle = handle;
  }, self)
 
  return o
end

function easy:handle()
  return self._handle
end

function easy:set(k, v, ...)
  local handle = self:handle()

  if type(k) == "table" then
    assert(v == nil)
    for k, v in pairs(k) do
      local ok, err = setopt(handle, k, v)
      if not ok then return nil, err, k end
    end
    return self
  end
 
  assert(type(k) == 'string')
  local ok, err = setopt(handle, k, v, ...)
  if not ok then return nil, err end
  return self
end

function easy:perform(opt)
  if opt then
    local ok, err = self:set(opt)
    if not ok then return nil, err end
  end

  local e = self:handle()

  local ok, err = e:perform()
  if ok then return nil, err end

  return self 
end

function easy:post(data)
  local post = curl.form()
  local ok, err
  for k, v in pairs(data) do
    if type(v) == "string" then
      ok, err = post:add_content(k, v)
    else
      assert(type(v) == "table")
      if v.data then
        ok, err = post:add_buffer(k, v.file, v.data, v.type, v.headers)
      else
        ok, err = post:add_file(k, v.file, v.data, v.type, v.filename, v.headers)
      end
    end
    if not ok then break end
  end
  if not ok then
    post:free()
    return nil, err
  end

  local e = self:handle()
  ok, err = e:setopt_httppost(post)
  if not ok then
    post:free()
    return nil, err
  end

  return self
end

function easy:get(k, ...)
  assert(type(k) == "string")
  local handle = self:handle()
  local fn = handle["getinfo_" .. k]
  if not fn then return nil, "Unknown info: " .. k end
  return fn(handle, ...)
end

end

local cURL = {}

cURL.easy = function(...) return easy:new(...) end

return cURL
