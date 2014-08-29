local cURL = require("lcurl")

local post = cURL.form()
  -- post file from filesystem
  :add_file  ("name", "post.lua", "text/plain")
  -- post file from data variable
  :add_buffer("name2", "dummy.html", "<html><bold>bold</bold></html>", "text/html")

cURL.easy()
  :setopt_url("http://localhost")
  :setopt_httppost(post)
  :perform()
:close()

-- Lua-cURL compatiable function
local function post(e, data)
  local form = cURL.form()
  local ok, err = true

  for k, v in pairs(data) do
    if type(v) == "string" then
      ok, err = form:add_content(k, v)
    else
      assert(type(v) == "table")
      if v.stream_length then
        form:free()
        error("Stream does not support")
      end
      if v.data then
        ok, err = form:add_buffer(k, v.file, v.data, v.type, v.headers)
      else
        ok, err = form:add_file(k, v.file, v.data, v.type, v.filename, v.headers)
      end
    end
    if not ok then break end
  end

  if not ok then
    form:free()
    return nil, err
  end

  ok, err = e:setopt_httppost(form)
  if not ok then
    form:free()
    return nil, err
  end

  return e
end

local e = cURL.easy()
  :setopt_url("http://localhost")

postdata = {  
  -- post file from filesystem
  name = {file="post.lua",
    type="text/plain"
  },
  -- post file from data variable
  name2 = {file="dummy.html",
    data="<html><bold>bold</bold></html>",
    type="text/html"
  },
}

post(e, postdata):perform()

print("Done")