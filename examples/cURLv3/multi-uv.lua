local curl = require "cURL"
local uv   = require "lluv"
local ut   = require "lluv.utils"

local fprintf = function(f, ...) f:write((string.format(...))) end
local printf  = function(...) fprintf(io.stdout, ...) end

local stderr = io.stderr

local trace = false

trace = trace and print or function() end

local ACTION_NAMES = {
  [curl.POLL_IN     ] = "POLL_IN";
  [curl.POLL_INOUT  ] = "POLL_INOUT";
  [curl.POLL_OUT    ] = "POLL_OUT";
  [curl.POLL_NONE   ] = "POLL_NONE";
  [curl.POLL_REMOVE ] = "POLL_REMOVE";
}

local EVENT_NAMES = {
  [ uv.READABLE ] = "READABLE";
  [ uv.WRITABLE ] = "WRITABLE";
  [ uv.READABLE + uv.WRITABLE ] = "READABLE + WRITABLE";
}

local FLAGS = {
  [ uv.READABLE               ] = curl.CSELECT_IN;
  [ uv.WRITABLE               ] = curl.CSELECT_OUT;
  [ uv.READABLE + uv.WRITABLE ] = curl.CSELECT_IN + curl.CSELECT_OUT;
}

local Context = ut.class() do

function Context:__init(fd)
  self._fd   = assert(fd)
  self._poll = uv.poll_socket(fd)
  self._poll.data = {context = self}

  assert(self._poll:fileno() == fd)

  return self
end

function Context:close()
  if not self._poll then return end
  self._poll.data = nil
  self._poll:close()
  self._poll, self._fd = nil
end

function Context:poll(...)
  self._poll:start(...)
end

function Context:fileno()
  return self._fd
end

end

-- Number of parallel request
local MAX_REQUESTS
local timer, multi
local qtask = ut.Queue.new() -- wait tasks
local qfree = ut.Queue.new() -- avaliable easy handles
local qeasy = {} -- all easy handles

local function cleanup()
  timer:close()

  for i, easy in ipairs(qeasy) do
    multi:remove_handle(easy)
    easy:close()
  end

  multi:close()
end

local proceed_queue, add_download do

proceed_queue = function()
  if qtask:empty() then return end

  if qfree:empty() then
    if #qeasy < MAX_REQUESTS then
      local easy = assert(curl.easy())
      qeasy[#qeasy + 1] = easy
      qfree:push(easy)
    else
      return
    end
  end

  local task = assert(qtask:pop())
  local url, num = task[1], task[2]

  local filename = tostring(num) ..  ".download"
  local file = io.open(filename, "w")
  if not file then
    fprintf(stderr, "Error opening %s\n", filename)
    return
  end

  local handle = assert(qfree:pop())

  handle:setopt{
    url           = url;
    writefunction = file;
  }

  handle.data = { file = file }

  multi:add_handle(handle)

  fprintf(stderr, "Added download %s -> %s\n", url, filename);
end

add_download = function(url, num)
  qtask:push{url, num}

  proceed_queue()
end

end

local on_libuv_poll, on_libuv_timeout

local on_curl_timeout, on_curl_action do

on_curl_timeout = function(ms)
  -- calls by curl --
  trace("CURL::TIMEOUT", ms)

  if ms <= 0 then ms = 1 end

  timer:start(ms, 0, on_libuv_timeout)
end

on_curl_action = function(easy, fd, action)
  local ok, err = pcall(function()
    trace("CURL::SOCKET", easy, s, ACTION_NAMES[action] or action)

    local context = easy.data.context
    if (action == curl.POLL_IN) or (action == curl.POLL_OUT) then
      if not context then
        context = Context.new(fd)
        easy.data.context = context
      end
    end

    assert(context:fileno() == fd)

    if     action == curl.POLL_IN  then  context:poll(uv.READABLE, on_libuv_poll)
    elseif action == curl.POLL_OUT then  context:poll(uv.WRITABLE, on_libuv_poll)
    elseif action == curl.POLL_REMOVE then
      if context then
        easy.data.context = nil
        context:close()
      end
    end
  end)

  if not ok then uv.defer(function() error(err) end) end
end

end

-- on_libuv_poll, on_libuv_timeout 
do

local curl_check_multi_info = function()
  while true do
    local easy, ok, err = multi:info_read(true)

    if not easy then
      multi:close()
      error(err)
    end

    if easy == 0 then break end

    local done_url = easy:getinfo_effective_url()

    local context = easy.data.context
    if context then context:close() end
    local file    = assert(easy.data.file)
    file:close()

    easy.data = nil
    qfree:push(easy)

    if ok then
      printf("%s DONE\n", done_url);
    elseif data == "error" then
      printf("%s ERROR - %s\n", done_url, tostring(err));
    end

    proceed_queue()
  end
end

on_libuv_poll = function(handle, err, events)
  trace("UV::POLL", handle, err, EVENT_NAMES[events] or events)

  local flags = assert(FLAGS[events], ("unknown event:" .. events))

  context = handle.data.context

  multi:socket_action(context:fileno(), flags)

  curl_check_multi_info()
end

on_libuv_timeout = function(timer)
  trace("UV::TIMEOUT", timer)

  local running_handles, err = multi:socket_action()

  curl_check_multi_info()
end

end

MAX_REQUESTS = 64

timer = uv.timer()

multi = curl.multi{
  timerfunction = on_curl_timeout;
  socketfunction = on_curl_action;
}

for i = 1, math.huge do
  local url = arg[i]
  if not url then break end
  add_download(url, i)
end

uv.run()

cleanup()
