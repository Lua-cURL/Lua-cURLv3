local cURL = require("cURL")

-- setup easy and url
c1 = cURL.easy{url = "http://www.lua.org/"}
c2 = cURL.easy{url = "http://luajit.org/"}

m = cURL.multi()
  :add_handle(c1)
  :add_handle(c2)

local f1 = io.open("lua.html",    "w+b")
local f2 = io.open("luajit.html", "w+b")

for data, type, easy in m:iperform() do
  if type == "data" and c1 == easy then f1:write(data) end
  if type == "data" and c2 == easy then f2:write(data) end
end
