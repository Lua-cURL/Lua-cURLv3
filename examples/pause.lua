local curl = require "lcurl"

local WAIT_COUNT   = 1

local RESOURCE_URL = "http://www.rfc-editor.org/rfc/rfc2543.txt"

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
    print('Pause')
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

local progress = 0
local function ProgressFunction()

  progress = progress + 1

  if state == State.WAIT then
    -- Wait until unpause
    waitCount = waitCount - 1
    if waitCount == 0 then
      state = State.WRITE
      print('Unpause :', progress)
      easy:pause(curl.PAUSE_CONT)
    end
  end

  -- WriteFunction written some data
  if state == State.WRITTEN then
    -- send pause to WriteFunction
    state = State.PAUSE
    progress = 0
  end

end

easy  = curl.easy{
  url              = RESOURCE_URL,
  accept_encoding  = "gzip,deflate",
  writefunction    = WriteFunction,
  progressfunction = ProgressFunction,
  noprogress       = false,
  followlocation   = true,
}

easy:perform()