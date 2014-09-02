local curl = require "lcurl"

-- returns size and reader
local function make_stream(ch, n, m)
  local size = n * m
  local i = -1
  return size, function()
    i = i + 1
    if i < m then 
      return (tostring(ch)):rep(n - 2) .. '\r\n'
    end
  end
end

local form = curl.form()
  :add_stream("test1",                            make_stream("a", 10, 4))
  :add_stream("test2", "test2.txt",               make_stream("b", 10, 4))
  :add_stream("test3", "test3.txt", "text/plain", make_stream("c", 10, 4))

curl.easy{
  url           = 'http://posttestserver.com/post.php',
  writefunction = io.write,
  httppost      = form,
  post          = true,
}:perform()

