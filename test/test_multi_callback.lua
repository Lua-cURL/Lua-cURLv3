local curl = require "lcurl"

local called, active_coroutine = 0

-- for Lua 5.1 compat
local function co_running()
  local co, main = coroutine.running()
  if main == true then return nil end
  return co
end

function on_timer()
  called = called + 1
  -- use `os.exit` because now Lua-cURL did not propogate error from callback
  if co_running() ~= active_coroutine then os.exit(-1) end
end

local function test_1()
  io.write('Test #1 - ')

  called, active_coroutine = 0

  local e = curl.easy()
  local m = curl.multi{ timerfunction = on_timer }

  active_coroutine = coroutine.create(function()
    m:add_handle(e)
  end)

  coroutine.resume(active_coroutine)
  assert(called == 1)

  active_coroutine = nil
  m:remove_handle(e)
  assert(called == 2)

  io.write('pass!\n')
end

local function test_2()
  io.write('Test #2 - ')

  called, active_coroutine = 0

  local e = curl.easy()
  local m = curl.multi{ timerfunction = on_timer }

  active_coroutine = coroutine.create(function()
    m:add_handle(e)
  end)

  coroutine.resume(active_coroutine)
  assert(called == 1)

  active_coroutine = coroutine.create(function()
    m:remove_handle(e)
  end)
  coroutine.resume(active_coroutine)
  assert(called == 2)

  io.write('pass!\n')
end

local function test_3()
  io.write('Test #3 - ')

  called, active_coroutine = 0

  local e = curl.easy()
  local m = curl.multi{ timerfunction = on_timer }

  active_coroutine = coroutine.create(function()
    m:add_handle(e)
  end)

  coroutine.resume(active_coroutine)
  assert(called == 1)

  active_coroutine = nil
  e:close()
  assert(called == 2)

  io.write('pass!\n')
end

local function test_4()
  io.write('Test #4 - ')

  called, active_coroutine = 0

  local e = curl.easy()
  local m = curl.multi{ timerfunction = on_timer }

  active_coroutine = coroutine.create(function()
    m:add_handle(e)
  end)

  coroutine.resume(active_coroutine)
  assert(called == 1)

  active_coroutine = coroutine.create(function()
    e:close()
  end)
  coroutine.resume(active_coroutine)
  assert(called == 2)

  io.write('pass!\n')
end

test_1()

test_2()

test_3()

test_4()
