local cURL = require("cURL")

local f1 = io.open("lua.html",    "w+b")
local f2 = io.open("luajit.html", "w+b")

-- setup easy and url
c1 = cURL.easy{url = "http://www.lua.org/", writefunction = f1}
c2 = cURL.easy{url = "http://luajit.org/",  writefunction = f2}
c3 = cURL.easy{url = "****://luajit.org/"} -- UNSUPPORTED_PROTOCOL

m = cURL.multi()
  :add_handle(c1)
  :add_handle(c2)
  :add_handle(c3)

local remain = 3
while remain > 0 do
  local last = m:perform() -- do some work
  if last < remain then    -- we have done some tasks
    while true do          -- proceed results/errors
      local e, ok, err = m:info_read(true) -- get result and remove handle
      if e == 0 then break end -- no more finished tasks
      if ok then -- succeed
        print(e:getinfo_effective_url(), '-', e:getinfo_response_code())
      else -- failure
        print(e:getinfo_effective_url(), '-', err)
      end
      e:close()
    end
  end
  remain = last

  -- wait while libcurl do io select
  m:wait()
end
