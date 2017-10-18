local function prequire(m)
  local ok, err = pcall(require, m)
  if not ok then return nil, err end
  return err
end

local uv      = prequire "lluv"
local Pegasus = require (uv and "lluv.pegasus" or "pegasus")

local server = Pegasus:new{host = '127.0.0.1', port = 7090}

server:start(function(request, response)
  response:statusCode(200)
  response:addHeader('Content-Type', 'text/plain')
  response:write('Hello from Pegasus')
end)

if uv then uv.run() end
