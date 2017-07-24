--
-- convert `debug.c` example from libcurl examples
--

local curl = require "lcurl"

local function printf(...)
  io.stderr:write(string.format(...))
end

local function dump(title, data, n)
  n = n or 16
  printf("%s, %10.10d bytes (0x%8.8x)\n", title, #data, #data)
  for i = 1, #data do
    if (i - 1) % n == 0 then printf("%4.4x: ", i-1) end
    printf("%02x ", string.byte(data, i, i))
    if i % n == 0 then printf("\n") end
  end
  if #data % n ~= 0 then printf("\n") end
end

local function my_trace(type, data)
  local text
  if type == curl.INFO_TEXT         then printf("== Info: %s", data) end
  if type == curl.INFO_HEADER_OUT   then text = "=> Send header"     end
  if type == curl.INFO_DATA_OUT     then text = "=> Send data"       end
  if type == curl.INFO_SSL_DATA_OUT then text = "=> Send SSL data"   end
  if type == curl.INFO_HEADER_IN    then text = "<= Recv header"     end
  if type == curl.INFO_DATA_IN      then text = "<= Recv data"       end
  if type == curl.INFO_SSL_DATA_IN  then text = "<= Recv SSL data"   end
  if text then dump(text, data) end
end
 
local easy = curl.easy{
  url            = "http://google.com",
  verbose        = true,
  debugfunction  = my_trace,
  followlocation = true,
  writefunction  = function()end,
}

easy:perform()
