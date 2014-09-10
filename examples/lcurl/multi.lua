local cURL = require("lcurl")

local f1 = assert(io.open("lua.html","a+"))

local f2 = assert(io.open("luajit.html","a+"))

c  = cURL.easy()
  :setopt_url("http://www.lua.org/")
  :setopt_writefunction(f1)

c2 = cURL.easy()
  :setopt_url("http://luajit.org/")
  :setopt_writefunction(f2)

m = cURL.multi()
  :add_handle(c)
  :add_handle(c2)

while m:perform() > 0 do
  m:wait()
end

print(m:info_read())
print(m:info_read())
