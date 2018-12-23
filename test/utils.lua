local curl = require "lcurl"

local function weak_ptr(val)
  return setmetatable({value = val},{__mode = 'v'})
end

local function gc_collect(n)
  for i = 1, (n or 2) do
    collectgarbage("collect")
  end
end

local function cver(min, maj, pat)
  return min * 2^16 + maj * 2^8 + pat
end

local function is_curl_ge(min, maj, pat)
  assert(0x070908 == cver(7,9,8))
  return curl.version_info('version_num') >= cver(min, maj, pat)
end

local function is_curl_eq(min, maj, pat)
  assert(0x070908 == cver(7,9,8))
  return curl.version_info('version_num') == cver(min, maj, pat)
end

local function read_file(n)
  local f, e = io.open(n, "r")
  if not f then return nil, e end
  local d, e = f:read("*all")
  f:close()
  return d, e
end

local function get_bin_by(str,n)
  local pos = 1 - n
  return function()
    pos = pos + n
    return (str:sub(pos,pos+n-1))
  end
end

local function stream(ch, n, m)
  return n, get_bin_by( (ch):rep(n), m)
end

local function Stream(ch, n, m)
  local size, reader

  local _stream = {}

  function _stream:read(...)
    _stream.called_ctx = self
    _stream.called_co  = coroutine.running()
    return reader(...)
  end

  function _stream:size()
    return size
  end

  function _stream:reset()
    size, reader = stream(ch, n, m)
    return self
  end

  return _stream:reset()
end

local function easy_dump_mime(easy, mime, url)
  local buffer = {}

  local function dump_mime(type, data)
    if type == curl.INFO_DATA_OUT then
      buffer[#buffer + 1] = data
    end
  end

  local ok, err = easy:setopt{
    url            = url or "http://127.0.0.1:7090";
    customrequest  = "GET";
    mimepost       = mime;
    verbose        = true;
    debugfunction  = dump_mime;
    writefunction  = function()end;
  }

  if not ok then return nil, err end

  ok, err = easy:perform()

  if not ok then return nil, err end

  return table.concat(buffer)
end

local function easy_dump_request(easy, url)
  local buffer = {}
  local headers = {}

  local function dump_mime(type, data)
    if type == curl.INFO_DATA_OUT then
      buffer[#buffer + 1] = data
    end

    if type == curl.INFO_HEADER_OUT then
      headers[#headers + 1] = data
    end
  end

  local ok, err = easy:setopt{
    url            = url or "http://127.0.0.1:7090";
    customrequest  = "GET";
    mimepost       = mime;
    verbose        = true;
    debugfunction  = dump_mime;
    writefunction  = function()end;
  }

  if not ok then return nil, err end

  ok, err = easy:perform()

  if not ok then return nil, err end

  return table.concat(buffer), table.concat(headers) 
end

local utils = {
  weak_ptr      = weak_ptr;
  gc_collect    = gc_collect;
  is_curl_ge    = is_curl_ge;
  is_curl_eq    = is_curl_eq;
  get_bin_by    = get_bin_by;
  read_file     = read_file;
  dump_mime     = easy_dump_mime;
  dump_request  = easy_dump_request;
  stream        = stream;
  Stream        = Stream;
}

utils.import = function(...)
  local n, t = select('#', ...), {}
  for i = 1, n do
    local name = select(i, ...)
    t[#t + 1] = assert(utils[name], 'unknown field: ' .. tostring(name))
  end
  return (unpack or table.unpack)(t, 1, n)
end

return utils