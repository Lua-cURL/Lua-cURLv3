local cURL = require("cURL")

-- setup easy 
c = cURL.easy_init()
c2 = cURL.easy_init()

-- setup url
c:setopt_url("http://www.lua.org/")
c2:setopt_url("http://luajit.org/")

m = cURL.multi_init()
m:add_handle(c)
m:add_handle(c2)

local f1 = io.open("lua.html","a+")
local f2 = io.open("luajit.html","a+")

for data, type, easy in m:perform() do
--    if (type == "header") then print(data) end
    if (type == "data" and c == easy) then f1:write(data) end
    if (type == "data" and c2 == easy) then f2:write(data) end
end