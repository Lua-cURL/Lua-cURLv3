local curl = require "cURL"
local uv   = require "lluv"

local fprintf = function(f, ...) f:write((string.format(...))) end
local printf  = function(...) fprintf(io.stdout, ...) end

local stderr = io.stderr

local timeout, curl_handle

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
  [ uv.READABLE ]               = curl.CSELECT_IN;
  [ uv.WRITABLE ]               = curl.CSELECT_OUT;
  [ uv.READABLE + uv.WRITABLE ] = curl.CSELECT_IN + curl.CSELECT_OUT;

}

local trace = true

trace = trace and print or function() end

local CONTEXT = {}

function create_curl_context(sockfd)
  local context = {
    sockfd      = sockfd;
    poll_handle = uv.poll_socket(sockfd);
  }
  context.poll_handle.data = context
  
  return context
end

function destroy_curl_context(context)
  context.poll_handle:close()
end 

function add_download(url, num)
  local filename = tostring(num) ..  ".download"
  local file = io.open(filename, "w")
  if not file then
    fprintf(stderr, "Error opening %s\n", filename)
    return
  end

  local handle = curl.easy{
    url           = url;
    writefunction = file;
  }

  handle.data = file

  curl_handle:add_handle(handle)
  fprintf(stderr, "Added download %s -> %s\n", url, filename);
end

function check_multi_info()
  while true do
    local easy, ok, err = curl_handle:info_read(true)
    if not easy then curl_handle:close() error(err) end
    if easy == 0 then break end

    local context = CONTEXT[e]
    if context then destroy_curl_context(context) end
    local file = assert(easy.data)
    file:close()
    local done_url = easy:getinfo_effective_url()
    easy:close()
    if ok then
      printf("%s DONE\n", done_url);
    elseif data == "error" then
      printf("%s ERROR - %s\n", done_url, tostring(err));
    end
  end
end

function curl_perform(handle, err, events)
  -- calls by libuv --
  trace("UV::POLL", handle, err, EVENT_NAMES[events] or events)

  local flags = assert(FLAGS[events], ("unknown event:" .. events))

  context = handle.data

  curl_handle:socket_action(context.sockfd, flags)

  check_multi_info()
end

function on_timeout(timer)
  -- calls by libuv --
  trace("UV::TIMEOUT", timer)

  local running_handles, err = curl_handle:socket_action()

  check_multi_info()
end

function start_timeout(timeout_ms)
  -- calls by curl --
  trace("CURL::TIMEOUT", timeout_ms)

  -- 0 means directly call socket_action, but we'll do it in a bit
  if timeout_ms <= 0 then timeout_ms = 1 end

  timeout:stop():start(timeout_ms, 0, on_timeout)
end

function handle_socket(easy, s, action)
  local ok, err = pcall(function()
    -- calls by curl --
    trace("CURL::SOCKET", easy, s, ACTION_NAMES[action] or action)

    local curl_context = CONTEXT[easy] or create_curl_context(s)
    CONTEXT[easy] = curl_context

    assert(curl_context.sockfd == s)

    if action == curl.POLL_IN then
      curl_context.poll_handle:start(uv.READABLE, curl_perform)
    elseif action == curl.POLL_OUT then
      curl_context.poll_handle:start(uv.WRITABLE, curl_perform)
    elseif action == curl.POLL_REMOVE then
      CONTEXT[easy] = nil
      destroy_curl_context(curl_context)
    end
  end)
  if not ok then uv.defer(function() error(err) end) end
end

timeout = uv.timer()

curl_handle = curl.multi{
  socketfunction = handle_socket;
  timerfunction  = start_timeout;
}

curl_handle = curl.multi{
  socketfunction = handle_socket;
  timerfunction  = start_timeout;
}

for i = 1, math.huge do
  local url = arg[i]
  if not url then break end
  add_download(url, i)
end

uv.run(loop, UV_RUN_DEFAULT)
