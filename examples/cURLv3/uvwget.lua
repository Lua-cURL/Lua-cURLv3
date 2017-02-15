--
-- implementation of uvwget example from
-- http://nikhilm.github.io/uvbook/index.html
--

local curl = require "cURL"
local uv   = require "lluv"
local ut   = require "lluv.utils"

local fprintf = function(f, ...) f:write((string.format(...))) end

local stderr = io.stderr

local trace = false do

trace = trace and print or function() end

end

local ACTION_NAMES = {
  [curl.POLL_IN     ] = "POLL_IN";
  [curl.POLL_INOUT  ] = "POLL_INOUT";
  [curl.POLL_OUT    ] = "POLL_OUT";
  [curl.POLL_NONE   ] = "POLL_NONE";
  [curl.POLL_REMOVE ] = "POLL_REMOVE";
}

local POLL_IO_FLAGS = {
  [ curl.POLL_IN    ] = uv.READABLE;
  [ curl.POLL_OUT   ] = uv.WRITABLE;
  [ curl.POLL_INOUT ] = uv.READABLE + uv.WRITABLE;
}

local EVENT_NAMES = {
  [ uv.READABLE               ] = "READABLE";
  [ uv.WRITABLE               ] = "WRITABLE";
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

local MAX_REQUESTS = 64      -- Number of parallel request
local timer, multi
local qtask = ut.Queue.new() -- wait tasks
local qfree = ut.Queue.new() -- avaliable easy handles
local qeasy = {}             -- all easy handles

local function on_begin(handle, url, num)
  local filename = tostring(num) ..  ".download"
  local file = io.open(filename, "w")
  if not file then
    fprintf(stderr, "Error opening %s\n", filename)
    return
  end
  handle.data.file = file
  handle:setopt_writefunction(file)

  fprintf(stderr, "Added download %s -> %s\n", url, filename);
  return true
end

local function on_end(handle, err, url)
  handle.data.file:close()
  handle.data.file = nil

  if err then
    fprintf(stderr, "%s ERROR - %s\n", url, tostring(err));
  else
    fprintf(stderr, "%s DONE\n", url);
  end
end

local function cleanup()
  timer:close()

  for i, easy in ipairs(qeasy) do
    multi:remove_handle(easy)
    if easy.data then
      local context = easy.data.context
      if context then context:close() end

      local file = easy.data.file
      if file then on_end(easy, 'closed', easy:getinfo_effective_url()) end
    end
    easy:close()
  end

  multi:close()
end

local proceed_queue, add_download do

proceed_queue = function()
  while true do
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

    local handle = assert(qfree:pop())

    handle:setopt{
      url           = url;
      fresh_connect = true;
      forbid_reuse  = true;
    }

    handle.data = {}

    if on_begin(handle, url, num) then
      multi:add_handle(handle)
    else
      handle:reset().data = nil
      qfree:push(handle)
    end
  end
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

  if not timer:active() then
    if ms <= 0 then ms = 1 end

    timer:start(ms, 0, on_libuv_timeout)
  end
end

on_curl_action = function(easy, fd, action)
  local ok, err = pcall(function()
    trace("CURL::SOCKET", easy, fd, ACTION_NAMES[action] or action)

    local context = easy.data.context

    local flag = POLL_IO_FLAGS[action]
    if flag then
      if not context then
        context = Context.new(fd)
        easy.data.context = context
      end
      context:poll(flag, on_libuv_poll)
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
    easy.data.context = nil

    if ok then on_end(easy, nil, done_url) else on_end(easy, err, done_url) end

    easy:reset()
    easy.data = nil
    qfree:push(easy)
  end

  proceed_queue()
end

on_libuv_poll = function(poller, err, events)
  trace("UV::POLL", poller, err, EVENT_NAMES[events] or events)

  local flags = assert(FLAGS[events], ("unknown event:" .. events))

  local context = poller.data.context

  multi:socket_action(context:fileno(), flags)

  curl_check_multi_info()
end

on_libuv_timeout = function(timer)
  trace("UV::TIMEOUT", timer)

  multi:socket_action()

  curl_check_multi_info()
end

end

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
