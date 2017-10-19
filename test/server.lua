local function prequire(m)
  local ok, err = pcall(require, m)
  if not ok then return nil, err end
  return err
end

local uv      = prequire "lluv-"
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

r:get('/get', function(request, response)
  local headers = request:headers()
  local params  = request:params()
  local path    = request:path()
  local ip      = request.ip

  local body, status = recvFullBody(request, 15)

  local result = json.encode({
    args    = params;
    headers = headers;
    origin  = ip;
    content = body;
    url     = 'http://127.0.0.1' .. path;
  }, {indent = true})

  response:statusCode(200)
  response:addHeader('Connection', 'close')
  response:contentType('application/json')
  response:write(result)
end)

r:get('/bytes/:size', function(request, response, params)
  local headers = request:headers()
  local size = tonumber(params.size) or 1024
  local result = rand_bytes(size)

  response:statusCode(200)
  response:contentType('application/octet-stream')
  response:write(result)
end)

r:post('/post', function(request, response, params)
  local headers = request:headers()
  local params  = request:params()
  local path    = request:path()
  local ip      = request.ip

  local body, status = recvFullBody(request, 15)

  local name, data, form = decode_form(body)
  if name then
    form = {[name] = data}
  else
    form = decode_params(body)
  end

  local result = json.encode({
    args    = params;
    headers = headers;
    origin  = ip;
    form    = form;
    url     = 'http://127.0.0.1' .. path;
  }, {indent = true})

  response:statusCode(200)
  response:addHeader('Connection', 'close')
  response:contentType('application/json')
  response:write(result)
end)

server:start(function(request, response)
  local headers = request:headers()

  response:statusCode(200)
  response:addHeader('Content-Type', 'text/plain')
  response:write('Hello from Pegasus')
end)

if uv then uv.run() end
