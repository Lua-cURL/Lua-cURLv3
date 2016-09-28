local curl = require "lcurl"

-- for Lua 5.1 compat
local function co_running()
  local co, main = coroutine.running()
  if main == true then return nil end
  return co
end

local state, called = true, 0
local thread, msg

local function check_thread()
  if thread ~= co_running() then
    print(msg)
    os.exit(-1)
  end
end

local m; m = curl.multi{
  timerfunction = function()
    check_thread()
    called = called + 1

    if state then state = false
      thread = coroutine.create(function()
        local e = curl.easy()
        m:add_handle(e)
      end)

      msg = 'add from coroutine'
      coroutine.resume(thread)
      assert(called == 2)

      msg, thread = 'add from main'
      local e = curl.easy()
      m:add_handle(e)
      assert(called == 3)
    end
  end
}

e = curl.easy()

m:add_handle(e)

assert(called == 3)
