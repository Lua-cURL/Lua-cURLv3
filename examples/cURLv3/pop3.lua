-- Simple pop3 wrapper
--
-- @usage
--  local mbox = pop3.new('pop3s://pop3.yandex.ru')
--
--  -- Yandex works only with tls
--  print('Open: ', mbox:open_tls('***', '***'))
--  print('NOOP: ', mbox:noop())
--  print('RETR: ', mbox:retr(1))
--
--  -- use message class from lua-pop3
--  -- https://github.com/moteus/lua-pop3
--  for k, msg in mbox:messages() do
--    print"----------------------------------------------"
--    print("subject:    ", msg:subject())
--    print("from:       ", msg:from())
--    print("to:         ", msg:to())
--    for i,v in ipairs(msg:full_content()) do
--      if v.text then  print("  ", i , "TEXT: ", v.type, #v.text)
--      else print("  ", i , "FILE: ", v.type, v.file_name or v.name, #v.data) end
--    end
--  end
--
--  mbox:close()
--

local cURL           = require "cURL.safe"
local find_ca_bundle = require "cURL.utils".find_ca_bundle
local message do local ok
  ok, message = pcall(require, "pop3.message")
  if not ok then message = nil end
end

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

local function pars_response(resp)
  local code, info = string.match(resp,"%s*(%S*)(.*)%s*")
  -- SASL GET ONLY "+"/"-"
  if code == '+OK' or code == '+' then
    return true, info
  elseif code == '-ERR' or code == '-' then
    return false, info
  end
  return nil, resp
end

local function split_2_numbers(data)
  local n1, n2 = string.match(data, "%s*(%S+)%s+(%S+)")
  return tonumber(n1), tonumber(n2)
end

local function split_1_number(data)
  local n1,s= string.match(data, "%s*(%S+)%s*(%S*)")
  return tonumber(n1),s
end

local crln = '\r\n'
local function writer(cb, ctx)
  local tail
  return function(str)
    -- @check Is libcurl guarantee thant data will be arraived line by line?
    -- if so we can just call `cb`
    if str then
      local t = split(tail and (tail .. str) or str, crln, true)
      if str:sub(-2) == crln then tail = nil
      else tail = table.remove(t) end
      for _, s in ipairs(t) do cb(s, ctx) end
    elseif tail then cb(tail, ctx) end
  end
end

local pop3 = {} do
pop3.__index = pop3

function pop3:new(host)
  return setmetatable({
    _url = assert(host)
  },self)
end

local function open(self, user, password, ssl)
  self._easy, err = cURL.easy{
    url            = self._url,
    username       = user,
    password       = password,
    customrequest  = 'NOOP',
    nobody         = true,
    headerfunction = function(h) self._response = h end
  }
  if not self._easy then return nil, err end

  if ssl then
    -- For AVAST
    -- http://www.avast.com/en-eu/faq.php?article=AVKB91#artTitle
    local cainfo, capath = find_ca_bundle('MailShield.crt')
    if not cainfo then
      -- On Windows try find ca_bundle
      cainfo, capath = find_ca_bundle()
    end
    local ok, err = self._easy:setopt{
      use_ssl       = cURL.USESSL_ALL,
      cainfo        = cainfo,
      capath        = capath,
    }
    if not ok then
      self:close()
      return nil, err
    end
  end

  local ok, err = self._easy:perform()
  if not ok then
    self:close()
    return nil, err
  end

  return self
end

local function exec(self)
  self._response = nil
  local ok, err = self._easy:perform()
  if not ok then return nil, err end
  assert(self._response)
  self._response = self._response:gsub("%s+$", "")
  return self:response()
end

local function exec_no_body(self, cmd, url)
  local ok, err = self._easy:setopt{
    url           = self._url .. (url and ("/" .. url) or ""),
    customrequest = cmd or '',
    nobody        = true,
  }
  if not ok then return nil, err end
  return exec(self)
end

local function exec_body_cb(self, cb, cmd, url)
  local ok, err = self._easy:setopt{
    url           = self._url .. (url and ("/" .. url) or ""),
    customrequest = cmd or '',
    nobody        = false,
    writefunction = writer(assert(cb))
  }
  if not ok then return nil, err end
  ok, err = exec(self)
  if not ok then return nil, err end
  return true
end

local function exec_body(self, cmd, url)
  local t = {}
  local ok, err = exec_body_cb(self,
    function(s) t[#t+1] = s end,
    cmd, url
  )
  if not ok then return nil, err end
  return t
end

local function exec_no_body_2_numbers(...)
  local ok, data = exec_no_body(...)
  if not ok then return ok, data end
  return split_2_numbers(data)
end

function pop3:open(user, password)
  return open(self, user, password, false)
end

function pop3:response()
  if self._response then
    return pars_response(self._response)
  end
end

function pop3:verbose(...)
  local ok, err
  if select("#", ...) == 0 then
    ok, err = self._easy:setopt_verbose(true)
  else
    ok, err = self._easy:setopt_verbose(not not ...)
  end
  if not ok then return nil, ok end
  return self
end

function pop3:open_tls(user, password)
  return open(self, user, password, true)
end

function pop3:noop()
  return exec_no_body(self, 'NOOP')
end

function pop3:stat()
  return exec_no_body_2_numbers(self, 'STAT')
end

function pop3:dele(id)
  return exec_no_body(self, 'DELE ' .. id)
end

function pop3:rset()
  return exec_no_body_2_numbers(self, 'RSET')
end

function pop3:list(id)
  if id then
    return exec_no_body_2_numbers(self, 'LIST ' .. id)
  end

  local t,i = {},0
  local fn = function(data)
    local no, size = split_2_numbers(data)
    if not (no and size) then
      return nil, "Wrong Response: `" .. data .. "`"
    end
    t[no] = size
    i = i + 1
  end

  local ok, err = exec_body_cb(self, fn, '')
  if not ok then return nil, err end
  if not ok then return nil, err end
  return t, i
end

function pop3:uidl(id)
  if id then
    local ok, data = exec_no_body(self, 'UIDL ' .. id)
    if not ok then return ok, data end
    local no, id = split_1_number(data)
    if not (no and id) then
      return nil, "Wrong Response:" .. data
    end
    return no,id
  end

  local t,i = {},0
  local fn = function(data)
    local no, id = split_1_number(data)
    if not (no and id) then
      return nil, "Wrong Response:" .. data
    end
    t[no]=id
    i = i + 1
    return true
  end

  local ok, err = exec_body_cb(self, fn, 'UIDL')
  if not ok then return nil, err end
  if not ok then return nil, err end
  return t, i
end

function pop3:retr(id)
  assert(id)
  return exec_body(self, '', tostring(id))
end

function pop3:top(id, n)
  assert(id)
  assert(type(n) == "number")
  return exec_body(self, 'TOP ' .. id .. ' ' .. n)
end

function pop3:make_iter(fn)
  local lst, err = self:list()
  if not lst then error(err) end
  local k = nil

  local iter
  iter = function ()
    k = next(lst, k)
    if not k then return nil end

    -- skip deleted messages ?
    local no, size = self:list(k)
    if no == false then return iter() end -- next message
    if not no then return error(size) end
    assert(no == k)

    local data, err = fn(self, k, size)
    if not data then error(err) end

    return k, data
  end

  return iter
end

if message then

function pop3:message(msgid)
  local msg, err = self:retr(msgid)
  if not msg then return nil, err end
  return message(msg)
end

end

function pop3:retrs()
  return self:make_iter(self.retr)
end

function pop3:tops(n)
  return self:make_iter(function(self, msgid)
    return self:top(msgid, n)
  end)
end

function pop3:messages()
  return self:make_iter(self.message)
end

function pop3:closed()
  return not not self._easy
end

function pop3:close()
  if self._easy then
    self._easy:close()
    self._easy = false
  end
end

end

return {
  new = function(...) return pop3:new() end;
}
