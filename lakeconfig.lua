local io = require "io"
io.stdout:setvbuf"no"
io.stderr:setvbuf"no"

function vc_version()
  local VER = lake.compiler_version()
  MSVC_VER = ({
    [15] = '9';
    [16] = '10';
  })[VER.MAJOR] or ''
  return MSVC_VER
end

if not L then

local function arkey(t)
  assert(type(t) == 'table')
  local keys = {}
  for k in pairs(t) do
    assert(type(k) == 'number')
    table.insert(keys, k)
  end
  table.sort(keys)
  return keys
end

local function ikeys(t)
  local keys = arkey(t)
  local i = 0
  return function()
    i = i + 1
    local k = keys[i]
    if k == nil then return end
    return k, t[k]
  end
end

local function expand(arr, t)
  if t == nil then return arr end

  if type(t) ~= 'table' then
    table.insert(arr, t)
    return arr
  end

  for _, v in ikeys(t) do
    expand(arr, v)
  end

  return arr
end

function L(...)
  return expand({}, {...})
end

end

J = J or path.join

IF = IF or lake.choose or choose

DIR_SEP = package.config:sub(1,1)

function prequire(...)
  local ok, mod = pcall(require, ...)
  if ok then return mod end
end

function clone(t, o)
  o = o or {}
  for k, v in pairs(t) do
    if o[k] == nil then o[k] = v end
  end
  return o
end

function each_join(dir, list)
  for i, v in ipairs(list) do
    list[i] = path.join(dir, v)
  end
  return list
end

function run(file, cwd)
  print()
  print("run " .. file)
  if not TESTING then
    if cwd then lake.chdir(cwd) end
    local status, code = utils.execute( LUA_RUNNER .. ' ' .. file )
    if cwd then lake.chdir("<") end
    print()
    return status, code
  end
  return true, 0
end

function exec(file, cwd)
  print()
  print("exec " .. file)
  if not TESTING then
    if cwd then lake.chdir(cwd) end
    local status, code = utils.execute( file )
    if cwd then lake.chdir("<") end
    print()
    return status, code
  end
  return true, 0
end

local TESTS = {}

function run_test(name, params)
  local test_dir = TESTDIR or J(ROOT, 'test')
  local cmd = J(test_dir, name)
  if params then cmd = cmd .. ' ' .. params end
  local ok = run(cmd, test_dir)

  table.insert(TESTS, {cmd = cmd, result = ok})

  print("TEST " .. name .. (ok and ' - pass!' or ' - fail!'))
end

function exec_test(name, params)
  local test_dir = TESTDIR or J(ROOT, 'test')
  local cmd = J(test_dir, name)
  if params then cmd = cmd .. ' ' .. params end
  local ok = exec(cmd, test_dir)

  table.insert(TESTS, {cmd = cmd, result = ok})

  print("TEST " .. name .. (ok and ' - pass!' or ' - fail!'))
end

function test_summary()
  local ok = true
  print("")
  print("------------------------------------")
  print("Number of tests:", #TESTS)
  for _, t in ipairs(TESTS) do
    ok = ok and t.result
    print((t.result and '  Pass' or '  Fail') .. " - TEST " .. t.cmd)
  end
  print("------------------------------------")
  print("")
  return ok
end

--[[spawn]] if WINDOWS then
  function spawn(file, cwd)
    local winapi = prequire "winapi"
    if not winapi then
      quit('needs winapi for spawn!')
      return false
    end

    print("spawn " .. file)
    if not TESTING then
      if cwd then lake.chdir(cwd) end
      assert(winapi.shell_exec(nil, LUA_RUNNER, file, cwd))
      if cwd then lake.chdir("<") end
      print()
    end
    return true
  end
else
  function spawn(file, cwd)
    print("spawn " .. file)
    if not TESTING then
      assert(run(file .. ' &', cwd))
    end
    return true
  end
end

function as_bool(v,d)
  if v == nil then return not not d end
  local n = tonumber(v)
  if n == 0 then return false end
  if n then return true end
  return false
end

--- set global variables 
-- LUA_NEED
-- LUA_DIR
-- LUA_RUNNER
-- ROOT
-- LUADIR
-- LIBDIR
-- TESTDIR
-- DOCDIR
-- DYNAMIC
function INITLAKEFILE()
  if LUA_VER == '5.3' then
    LUA_NEED   = 'lua53'
    LUA_DIR    = ENV.LUA_DIR_5_3 or ENV.LUA_DIR
    LUA_RUNNER = LUA_RUNNER or 'lua53'
  elseif LUA_VER == '5.2' then
    LUA_NEED   = 'lua52'
    LUA_DIR    = ENV.LUA_DIR_5_2 or ENV.LUA_DIR
    LUA_RUNNER = LUA_RUNNER or 'lua52'
  elseif LUA_VER == '5.1' then
    LUA_NEED   = 'lua51'
    LUA_DIR    = ENV.LUA_DIR
    LUA_RUNNER = LUA_RUNNER or 'lua'
  else
    LUA_NEED   = 'lua'
    LUA_DIR    = ENV.LUA_DIR
    LUA_RUNNER = LUA_RUNNER or 'lua'
  end
  ROOT    = ROOT    or J( LUA_DIR, 'libs', PROJECT )
  LUADIR  = LUADIR  or J( ROOT,    'share'         )
  LIBDIR  = LIBDIR  or J( ROOT,    'share'         )
  TESTDIR = TESTDIR or J( ROOT,    'test'          )
  DOCDIR  = DOCDIR  or J( ROOT,    'doc'           )
  DYNAMIC = as_bool(DYNAMIC, false)
end

-----------------------
-- needs --
-----------------------

lake.define_need('lua53', function()
  return {
    incdir = J(ENV.LUA_DIR_5_3, 'include');
    libdir = J(ENV.LUA_DIR_5_3, 'lib');
    libs = {'lua53'};
  }
end)

lake.define_need('lua52', function()
  return {
    incdir = J(ENV.LUA_DIR_5_2, 'include');
    libdir = J(ENV.LUA_DIR_5_2, 'lib');
    libs = {'lua52'};
  }
end)

lake.define_need('lua51', function()
  return {
    incdir = J(ENV.LUA_DIR, 'include');
    libdir = J(ENV.LUA_DIR, 'lib');
    libs = {'lua5.1'};
  }
end)

local CURL_DIR = CURL_DIR or ENV.CURL_DIR or J(ENV.CPPLIB_DIR, 'curl', '7.37.1')

lake.define_need('libcurl', function()
  return {
    incdir = J(CURL_DIR, 'include');
    libdir = J(CURL_DIR, 'lib');
    libs   = {'libcurl'};
  }
end)

lake.define_need('winsock2', function()
  return {libs = {"ws2_32"}}
end)
