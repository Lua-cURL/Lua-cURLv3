local RUN = lunit and function()end or function ()
  local res = lunit.run()
  if res.errors + res.failed > 0 then
    os.exit(-1)
  end
  return os.exit(0)
end

lunit = require "lunit"

local ok, curl = pcall(require, "cURL")
local version if ok then
  version = curl.version()
else
  io.stderr:write('can not load cURL:' .. curl)
  os.exit(-1)
end

print("------------------------------------")
print("Module    name: " .. curl._NAME);
print("Module version: " .. curl._VERSION);
print("Lua    version: " .. (_G.jit and _G.jit.version or _G._VERSION))
print("cURL   version: " .. version)
print("------------------------------------")
print("")

require "test_safe"
require "test_easy"
require "test_form"
require "test_curl"

RUN()
