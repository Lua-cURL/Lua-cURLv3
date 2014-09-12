local cURL = require "cURL"

-- Stream class
local Stream = {} do
Stream.__index = Stream

function Stream:new(ch, length)
  local o = setmetatable({}, self)
  o._len   = length
  o._ch    = ch
  o._pos   = 0
  o._step  = 7
  return o
end

function Stream:length()
  return self._len
end

function Stream:read()
  local n = self._len - self._pos
  if n <= 0 then return '' end -- eof
  if n > self._step then n = self._step end
  self._pos = self._pos + n
  return self._ch:rep(n)
end

end

-- returns size and reader
local function make_stream(ch, n, m)
  local size = n * m
  local i = -1
  return size, function()
    i = i + 1
    if i < m then 
      return (tostring(ch)):rep(n - 2) .. '\r\n'
    end
    return nil
  end
end

local length, stream = make_stream("a", 10, 4)

c = cURL.easy{
  url      = "http://posttestserver.com/post.php",
  -- url      = "http://httpbin.org/post",
  post     = true,
  httppost = cURL.form{

    -- file form filesystem
    name01 = {
      file = "post_form.lua",
      type = "text/plain",
      name = "post.lua",
    },

    -- file form string
    name02 = {
      data = "<html><bold>bold</bold></html>",
      name = "dummy.html",
      type = "text/html",
    },

    -- file form stream object
    name03 = {
      stream  = Stream:new('8', 25),
      name    = "stream1.txt",
      type    = "text/plain",
      headers = {
        "X-Test-Char : 8",
        "X-Test-length : 25",
      }
    },

    -- file form stream function
    name04 = {
      stream  = stream,
      length  = length,
      name    = "stream2.txt",
      type    = "text/plain",
    },

  },
}

c:perform()
