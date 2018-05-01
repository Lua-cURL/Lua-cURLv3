local curl = require "lcurl"

local function keys(t)
  local s = {}
  for k in pairs(t) do
    s[#s + 1] = k
  end
  table.sort(s)
  return s
end

local function printf(...)
  return print(string.format(...))
end

local exclude = {protocols = 1, features = 1, version = 1, version_num = 1}

local info = curl.version_info()

print(curl.version())

for _, key in ipairs(keys(info)) do if not exclude[key] then
  printf("%15s: %s", key, info[key])
end end

print('Protocols:')
for _, protocol in ipairs(keys(info.protocols))do
  local on = info.protocols[protocol]
  printf('  [%s] %s', on and '+' or '-', protocol)
end

print('Features:')
for _, feature in ipairs(keys(info.features))do
  local on = info.features[feature]
  printf('  [%s] %s', on and '+' or '-', feature)
end
