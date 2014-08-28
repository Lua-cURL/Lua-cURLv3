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

print("Done")