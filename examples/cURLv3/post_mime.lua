local curl = require "cURL"

-- Documentation does not restrict using the same mime with different easy handle.
-- In fact it does not mention this case at all. But in my experiments it works.
local easy = curl.easy()

-- Create new mime object.
local mime = easy:mime()

-- Add part. Source of data is file on disk.
-- If file not exists then perform returns `[CURL-EASY][READ_ERROR]` error
-- There no way to remove existed part from mime.
local part = mime:addpart{
  filedata = './post_mime.lua',
  type     = 'application/lua',
  filename = 'post_mime.lua',
  encoder  = 'base64',
}

local buffer = {}

-- Use mime object as request body
easy:setopt{
  url      = 'http://127.0.0.1:7090/post',
  mimepost = mime,
}

easy:setopt_writefunction(table.insert, buffer)

local ok, err = easy:perform()
if ok then
  local code, url, content =  easy:getinfo_effective_url(),
      easy:getinfo_response_code(), table.concat(buffer)
  print(code, url)
  print(content)
else
  print(err)
end

easy:close()

-- Lua-cURL does not free mime when close `parent` easy.

-- Explicitly free mime object and all its parts
-- There no way to reuse only some parts or submimes.
-- but it possible reuse entire mime again.
mime:free()
