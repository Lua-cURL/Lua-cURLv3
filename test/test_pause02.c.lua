local curl = require "lcurl"

-- How many times curl_multi_perform should be called before hitting of CURLPAUSE_CONT.
-- (including curl_multi_perform that causes WriteFunction to pause writes,
-- i.e. 1 means that CURLPAUSE_CONT will be performed immediately after pause.)
local WAIT_COUNT   = 15

local SIZE         = 10 * 1024
local RESOURCE_URL = "http://httpbin.org/bytes/" .. SIZE

local State = {
  PAUSE   = 0, -- write function should return CURL_WRITEFUNC_PAUSE
  WAIT    = 1, -- waiting for CURLPAUSE_CONT
  WRITE   = 2, -- write function should perform write
  WRITTEN = 3, -- write function have performed write
}

-- Current state
local state = State.PAUSE

-- Countdown to continue writes
local waitCount = 0

-- Received data and data size
local data, datasize = {}, 0

local function WriteFunction(str)
  if state == State.PAUSE then
    state = State.WAIT
    waitCount = WAIT_COUNT
    return curl.WRITEFUNC_PAUSE
  end

  if state == State.WAIT then
    -- callback shouldn't be called in this state
    print("WARNING: write-callback called in STATE_WAIT")
    return curl.WRITEFUNC_PAUSE
  end

  if state == State.WRITE then
    state = State.WRITTEN
  end

  datasize = datasize  + #str
  data[#data + 1] = str
end

local function perform(multi, easy)
  while true do
    local handles = multi:perform()

    if state == State.WAIT then
      waitCount = waitCount - 1
      if waitCount == 0 then
        state = State.WRITE
        easy:pause(curl.PAUSE_CONT)
      end
    end

    if state == State.WRITTEN then
      state = State.PAUSE
    end

    if 0 == handles then
      local h, ok, err = multi:info_read()
      return not not ok, err
    end

    multi:wait()
  end
end

local easy  = curl.easy{
  url             = RESOURCE_URL,
  accept_encoding = "gzip,deflate",
  writefunction   = WriteFunction,
}
local multi = curl.multi()
multi:add_handle(easy)

local ok, err = perform(multi, easy)

if ok then
  print("OK: data retrieved successfully (" .. tostring(datasize) .. ")")
else
  print("ERROR: data retrieve failed (" .. tostring(err) .. ")")
  os.exit(1)
end
