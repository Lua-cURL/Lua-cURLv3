-- Simple pop3 wrapper
--
-- @usage
--  local mbox = pop3:new('pop3://pop3.yandex.ru')
--
--  -- Yandex works only with tls
--  print('Open: ', mbox:open_tls('***', '***'))
--  print('NOOP: ', mbox:noop())
--  print('RETR: ', mbox:retr(1))
--
--  list = mbox:list()
--  for no, size in ipairs(list)do
--  ...
--  end
--
--  mbox:close()
--

local cURL = require "cURL.safe"

local find_ca_bundle = require "cURL.utils".find_ca_bundle

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

local crln = '\r\n'
local function writer(cb, ctx)
  local tail
  return function(str)
    if str then
      local t = split(tail and (tail .. str) or str, crln, true)
      if str:sub(-2) == crln then tail = nil
      else tail = table.remove(t) end
      for _, s in ipairs(t) do cb(ctx, s) end
    elseif tail then cb(ctx, tail) end
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
    url           = self._url,
    username      = user,
    password      = password,
    customrequest = 'NOOP',
    nobody        = true,
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

function pop3:open(user, password)
  return open(self, user, password, false)
end

function pop3:open_tls(user, password)
  return open(self, user, password, true)
end

function pop3:noop()
  local ok, err = self._easy:setopt{
    url           = self._url,
    customrequest = 'NOOP',
    nobody        = true,
  }
  if not ok then return nil, err end
  ok, err = self._easy:perform()
  if not ok then return nil, err end
  return self
end

function pop3:list()
  local t = {}
  local ok, err = self._easy:setopt{
    url           = self._url,
    customrequest = '',
    nobody        = false,
    writefunction = writer(function(t, s) t[#t+1] = s end, t)
  }
  if not ok then return nil, err end

  ok, err = self._easy:perform()

  if not ok then return nil, err end
  return t
end

function pop3:retr(n)
  local t = {}
  local ok, err = self._easy:setopt{
    url           = self._url .. '/' .. n,
    customrequest = '',
    nobody        = false,
    writefunction = writer(function(t, s) t[#t+1] = s end, t)
  }
  if not ok then return nil, err end

  ok, err = self._easy:perform()

  if not ok then return nil, err end
  return t
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
