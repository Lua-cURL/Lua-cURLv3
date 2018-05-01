local function prequire(m)
  local ok, err = pcall(require, m)
  if not ok then return nil, err end
  return err
end

local uv      = prequire "lluv"
local Pegasus = require (uv and "lluv.pegasus" or "pegasus")
local Router  = require "pegasus.plugins.router"
local json    = require "dkjson"
-- local pp      = require "pp"

local function decode_form(form)
  return string.match(form, '\r\nContent%-Disposition:%s*form%-data;%s*name="(.-)".-\r\n\r\n(.-)\r\n')
end

local function decode_params(str)
  local params = {}
  for k, v in string.gmatch(str, '([^=]+)=([^&]+)&?') do
    params[k] = v
  end
  return params
end

local function rand_bytes(n)
  local res = {}
  for i = 1, n do
    res[#res + 1] = string.char(math.random(254))
  end
  return table.concat(res)
end

local r = Router:new()

local server = Pegasus:new{
  plugins = { r };
  host = '127.0.0.1', port = 7090, timout = 10
}

local function recvFullBody(request, T1)
  local body, counter = {}, 0

  local result, status
  while true do
    result, status = request:receiveBody()
    if result then
      counter = 0
      body[#body + 1] = result
    elseif status ~= 'timeout' then
      break
    else
      counter = counter + 1
      if counter > T1 then break end
    end
  end

  return table.concat(body), status
end

local function buildResponse(request)
  local headers = request:headers()
  local params  = request:params()
  local path    = request:path()
  local ip      = request.ip
  local host    = headers and headers.Host or '127.0.0.1'
  local url     = string.format('http://%s%s', host, path)

  return {
    args    = params;
    headers = headers;
    origin  = ip;
    url     = url;
  }
end

r:get('/get', function(request, response)
  local result = buildResponse(request)
  result.body = recvFullBody(request, 15)

  response:statusCode(200)
  response:contentType('application/json')
  response:write(json.encode(result, {indent = true}))
end)

r:post('/post', function(request, response, params)
  local result = buildResponse(request)

  local body, status = recvFullBody(request, 15)

  local name, data = decode_form(body)
  if name then
    result.form = {[name] = data}
  else
    result.form = decode_params(body)
  end

  response:statusCode(200)
  response:contentType('application/json')
  response:write(json.encode(result, {indent = true}))
end)

r:get('/bytes/:size', function(request, response, params)
  local headers = request:headers()
  local size = tonumber(params.size) or 1024
  local result = rand_bytes(size)

  response:statusCode(200)
  response:addHeader('Connection', 'close')
  response:contentType('application/octet-stream')
  response:write(result)
end)

server:start(function(request, response)
  local headers = request:headers()

  response:statusCode(200)
  response:addHeader('Content-Type', 'text/plain')
  response:write('Hello from Pegasus')
end)

if uv then uv.run() end
