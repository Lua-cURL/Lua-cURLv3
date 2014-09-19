local RUN = lunit and function()end or function ()
  local res = lunit.run()
  if res.errors + res.failed > 0 then
    os.exit(-1)
  end
  return os.exit(0)
end

lunit = require "lunit"

local ok, curl = pcall(require, "lcurl")
local version if ok then
  version = curl.version()
else
  version = "<UNKNOWN>"
end

print("------------------------------------")
print("Lua  version: " .. (_G.jit and _G.jit.version or _G._VERSION))
print("cURL version: " .. version)
print("------------------------------------")
print("")

require "test_safe"
require "test_easy"
require "test_form"
require "test_curl"

RUN()
