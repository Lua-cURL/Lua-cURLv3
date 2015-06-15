local lunit, RUN = lunit do
RUN   = lunit and function()end or function ()
  local res = lunit.run()
  if res.errors + res.failed > 0 then
    os.exit(-1)
  end
  return os.exit(0)
end
lunit = require "lunit"
end

local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end

local curl       = require "lcurl"
local scurl      = require "lcurl.safe"
local json       = require "dkjson"
local path       = require "path"
local upath      = require "path".new('/')
local url        = "http://example.com"
local fname      = "./test.download"

print("------------------------------------")
print("Lua  version: " .. (_G.jit and _G.jit.version or _G._VERSION))
print("cURL version: " .. curl.version())
print("------------------------------------")
print("")

local function weak_ptr(val)
  return setmetatable({value = val},{__mode = 'v'})
end

local function gc_collect()
  collectgarbage("collect")
  collectgarbage("collect")
end

local function cver(min, maj, pat)
  return min * 2^16 + maj * 2^8 + pat
end

local function is_curl_ge(min, maj, pat)
  assert(0x070908 == cver(7,9,8))
  return curl.version_info('version_num') >= cver(min, maj, pat)
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

local function strem(ch, n, m)
  return n, get_bin_by( (ch):rep(n), m)
end

local ENABLE = true

local _ENV = TEST_CASE'write_callback'       if ENABLE then

local c, f

function teardown()
  if f then f:close() end
  os.remove(fname)
  if c then c:close() end
  f, c = nil
end

function test_write_to_file()
  f = assert(io.open(fname, "w+b"))
  c = assert(curl.easy{
    url = url;
    writefunction = f;
  })

  assert_equal(c, c:perform())
end

function test_write_abort_01()
  c = assert(scurl.easy{
    url = url;
    writefunction = function(str) return #str - 1 end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_write_abort_02()
  c = assert(scurl.easy{
    url = url;
    writefunction = function(str) return false end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_write_abort_03()
  c = assert(scurl.easy{
    url = url;
    writefunction = function(str) return nil, "WRITEERROR" end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal("WRITEERROR", e)
end

function test_write_abort_04()
  c = assert(scurl.easy{
    url = url;
    writefunction = function(str) return nil end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_reset_write_callback()
  f = assert(io.open(fname, "w+b"))
  c = assert(curl.easy{url = url})
  assert_equal(c, c:setopt_writefunction(f))
  assert_equal(c, c:setopt_writefunction(f.write, f))
  assert_equal(c, c:setopt_writefunction(print))
  assert_error(function()c:setopt_writefunction()end)
  assert_error(function()c:setopt_writefunction(nil)end)
  assert_error(function()c:setopt_writefunction(nil, f)end)
end

function test_write_pass_01()
  c = assert(curl.easy{
    url = url;
    writefunction = function(s) return #s end
  })

  assert_equal(c, c:perform())
end

function test_write_pass_02()
  c = assert(curl.easy{
    url = url;
    writefunction = function() return  end
  })

  assert_equal(c, c:perform())
end

function test_write_pass_03()
  c = assert(curl.easy{
    url = url;
    writefunction = function() return true end
  })

  assert_equal(c, c:perform())
end

end

local _ENV = TEST_CASE'progress_callback'    if ENABLE then

local c

local function pass() end

function teardown()
  if f then f:close() end
  os.remove(fname)
  if c then c:close() end
  f, c = nil
end

function test_abort_01()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return false end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_02()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return 0 end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_03()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return nil end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_04()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return nil, "PROGRESSERROR" end
  })

  local _, e = assert_nil(c:perform())
  assert_equal("PROGRESSERROR", e)
end

function test_abort_05()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() error( "PROGRESSERROR" )end
  })

  assert_error_match("PROGRESSERROR", function()
    c:perform()
  end)
end

function test_pass_01()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() end
  })

  assert_equal(c, c:perform())
end

function test_pass_02()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return true end
  })

  assert_equal(c, c:perform())
end

function test_pass_03()
  c  = assert(scurl.easy{
    url              = url,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return 1 end
  })

  assert_equal(c, c:perform())
end

end

local _ENV = TEST_CASE'header_callback'      if ENABLE then

local c, f
local function dummy() end

function teardown()
  if c then c:close() end
  f, c = nil
end

function test_header_abort_01()
  c = assert(scurl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function(str) return #str - 1 end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_header_abort_02()
  c = assert(scurl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function(str) return false end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_header_abort_03()
  c = assert(scurl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function(str) return nil, "WRITEERROR" end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal("WRITEERROR", e)
end

function test_header_abort_04()
  c = assert(scurl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function(str) return nil end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_reset_header_callback()
  f = {header = function() end}
  c = assert(curl.easy{url = url})
  assert_equal(c, c:setopt_headerfunction(f))
  assert_equal(c, c:setopt_headerfunction(f.header, f))
  assert_equal(c, c:setopt_headerfunction(print))
  assert_error(function()c:setopt_headerfunction()end)
  assert_error(function()c:setopt_headerfunction(nil)end)
  assert_error(function()c:setopt_headerfunction(nil, f)end)
end

function test_header_pass_01()
  c = assert(curl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function(s) return #s end
  })

  assert_equal(c, c:perform())
end

function test_header_pass_02()
  c = assert(curl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function() return  end
  })

  assert_equal(c, c:perform())
end

function test_header_pass_03()
  c = assert(curl.easy{
    url = url;
    writefunction = dummy,
    headerfunction = function() return true end
  })

  assert_equal(c, c:perform())
end


end

local _ENV = TEST_CASE'read_stream_callback' if ENABLE and is_curl_ge(7,30,0) then

-- tested on WinXP(x32)/Win8(x64) libcurl/7.37.1 / libcurl/7.30.0

local url = "http://httpbin.org/post"

local c, f, t

local function json_data()
  return json.decode(table.concat(t))
end

function setup()
  t = {}
  f = assert(scurl.form())
  c = assert(scurl.easy{
    url           = url,
    timeout       = 60,
  })
  assert_equal(c, c:setopt_writefunction(table.insert, t))
end

function teardown()
  if f then f:free() end
  if c then c:close() end
  t, f, c = nil
end

function test()
  assert_equal(f, f:add_stream('SSSSS', strem('X', 128, 13)))
  assert_equal(c, c:setopt_httppost(f))
  assert_equal(c, c:perform())
  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())
  assert_table(data.form)
  assert_equal(('X'):rep(128), data.form.SSSSS)
end

function test_abort_01()
  assert_equal(f, f:add_stream('SSSSS', 128 * 1024, function() end))
  assert_equal(c, c:setopt_timeout(5))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_OPERATION_TIMEDOUT), e)
end

function test_abort_02()
  assert_equal(f, f:add_stream('SSSSS', 128, function() return nil, "READERROR" end))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal("READERROR", e)
end

function test_abort_03()
  assert_equal(f, f:add_stream('SSSSS', 128, function() return 1 end))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_04()
  assert_equal(f, f:add_stream('SSSSS', 128, function() return true end))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_05()
  assert_equal(f, f:add_stream('SSSSS', 128, function() error("READERROR") end))
  assert_equal(c, c:setopt_httppost(f))

  assert_error_match("READERROR", function() c:perform() end)
end

function test_abort_06()
  assert_equal(f, f:add_stream('SSSSS', 128, function() return false end))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_pass_01()
  assert_equal(c, c:setopt_timeout(10))
  assert_equal(f, f:add_stream('SSSSS', 128, function() return nil end))
  assert_equal(c, c:setopt_httppost(f))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_OPERATION_TIMEDOUT), e)
end

function test_pause()
  local counter = 0
  assert_equal(f, f:add_stream('SSSSS', 128, function() 
    if counter == 0 then
      counter = counter + 1
      return curl.READFUNC_PAUSE
    end
    if counter == 1 then
      counter = counter + 1
      return ('X'):rep(128)
    end
    return ''
  end))

  assert_equal(c, c:setopt_progressfunction(function()
    if counter == 1 then
      c:pause(curl.PAUSE_CONT)
    end
  end))

  assert_equal(c, c:setopt_noprogress(false))

  assert_equal(c, c:setopt_httppost(f))

  assert_equal(c, c:perform())
  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())
  assert_table(data.form)
  assert_equal(('X'):rep(128), data.form.SSSSS)
end

end

local _ENV = TEST_CASE'read_callback'        if ENABLE then

local uname = upath:normolize(path.fullpath(fname))

local url   = "FILE://" .. uname

local c

function setup()
  c = assert(scurl.easy{
    url           = url,
    upload        = true,
  })
end

function teardown()
  os.remove(fname)
  if c then c:close() end
  c = nil
end

function test_abort_01()
--  assert_equal(c, c:setopt_readfunction(function() end))
--
--  local _, e = assert_nil(c:perform())
--  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_02()
  assert_equal(c, c:setopt_readfunction(function() return nil, "READERROR" end))

  local _, e = assert_nil(c:perform())
  assert_equal("READERROR", e)
end

function test_abort_03()
  assert_equal(c, c:setopt_readfunction(function() return 1 end))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_04()
  assert_equal(c, c:setopt_readfunction(function() return true end))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_05()
  assert_equal(c, c:setopt_readfunction(function() error("READERROR") end))

  assert_error_match("READERROR", function() c:perform() end)
end

function test_abort_06()
  assert_equal(c, c:setopt_readfunction(function() return false end))

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_pause()

-- BUG?
-- c:perform() returns curl.E_READ_ERROR after readfunction return curl.READFUNC_PAUSE
--
-- OS   version        : Linux Mint 17 (x86_64)
-- cURL version        : libcurl/7.35.0 OpenSSL/1.0.1f zlib/1.2.8 libidn/1.28 librtmp/2.3
-- version_info("host"): x86_64-pc-linux-gnu
--
-- OS   version        : Windows XP (x86_64)
-- cURL version        : libcurl/7.38.0 OpenSSL/1.0.1c zlib/1.2.7 WinIDN
-- cURL version        : libcurl/7.37.1 OpenSSL/1.0.1c zlib/1.2.7 WinIDN
-- version_info("host"): i386-pc-win32
--
-- Works correctly on
-- (same binary as with libcurl 7.38.0/7.37.1)
--
-- OS   version        : Windows XP (x86_64)
-- cURL version        : libcurl/7.30.0 OpenSSL/0.9.8y zlib/1.2.7
-- version_info("host"): i386-pc-win32
--

  local counter = 0
  assert_equal(c, c:setopt_readfunction(function() 
    if counter == 0 then
      counter = counter + 1
      return curl.READFUNC_PAUSE
    end
    if counter == 1 then
      counter = counter + 1
      return ('X'):rep(128)
    end
    return ''
  end))

  assert_equal(c, c:setopt_progressfunction(function()
    if counter == 1 then
      c:pause(curl.PAUSE_CONT)
    end
  end))

  assert_equal(c, c:setopt_noprogress(false))

  local ok, err = c:perform()

  if (not ok) and err:name() == "READ_ERROR" then
    skip("TODO check pause on readfunction")
  end

  assert_equal(c, ok, err)

  assert_equal(0, c:getinfo_response_code())
end

function test_readbuffer()
  local flag = false
  local N
  assert_equal(c, c:setopt_readfunction(function(n)
    if not flag then
      flag = true
      N = math.floor(n*2 + n/3)
      assert(N > n)
      return ("s"):rep(N)
    end
    return ''
  end))

  assert_equal(c, c:perform())
  c:close()
  local data = read_file(fname)
  assert_equal(N, #data)
  assert_equal(("s"):rep(N), data)
end

function test_pass_01()
  -- We need this to support file:read() method which returns nil as EOF
  assert_equal(c, c:setopt_readfunction(function() return nil end))

  assert_equal(c, c:perform())
  c:close()
  local data = read_file(fname)
  assert_equal(0, #data)
end

function test_pass_02()
  local counter = 10
  assert_equal(c, c:setopt_readfunction(function()
    if counter > 0 then
      counter = counter - 1
      return 'a'
    end
  end))

  assert_equal(c, c:perform())
  c:close()
  local data = read_file(fname)
  assert_equal(('a'):rep(10), data)
end

end

local _ENV = TEST_CASE'escape'               if ENABLE then

local c

function teardown()
  if c then c:close() end
  f, c = nil
end

function test()
  local e = "This%2Bis%2Ba%2Bsimple%2B%2526%2Bshort%2Btest."
  local d = "This+is+a+simple+%26+short+test."
  c = assert(curl.easy())
  assert_equal(e, c:escape(d))
  assert_equal(d, c:unescape(e))
end

end

local _ENV = TEST_CASE'setopt_form'          if ENABLE then

local c

function teardown()
  if c then c:close() end
  c = nil
end

function test()
  local pfrom, e
  do
    local form = curl.form()
    e = curl.easy{httppost = form}
    pfrom = weak_ptr(form)
  end

  gc_collect()
  assert(pfrom.value)

  e:setopt_httppost(curl.form())

  gc_collect()
  assert(not pfrom.value)
end

function test_unset()
  local pfrom, e
  do
    local form = curl.form()
    e = curl.easy{httppost = form}
    pfrom = weak_ptr(form)
  end

  gc_collect()
  assert(pfrom.value)

  e:unsetopt_httppost()

  gc_collect()
  assert(not pfrom.value)
end

function test_reset()
  local pfrom, e
  do
    local form = curl.form()
    e = curl.easy{httppost = form}
    pfrom = weak_ptr(form)
  end

  gc_collect()
  assert(pfrom.value)

  e:reset()

  gc_collect()
  assert(not pfrom.value)
end

end

local _ENV = TEST_CASE'setopt_postfields'    if ENABLE then

local c

function teardown()
  if c then c:close() end
  c = nil
end

function test()

  do local fields = {}
    for i = 1, 100 do fields[#fields + 1] = "key" .. i .. "=value"..i end
    fields = table.concat(fields, '&')
    c = assert(curl.easy{
      url           = "http://httpbin.org/post",
      postfields    = fields,
      writefunction = function()end,
    })
  end

  -- call gc to try clear `fields` string
  for i = 1, 4 do collectgarbage"collect" end

  c:perform()
end

function test_unset()
  local pfields

  do local fields = {}
    for i = 1, 100 do fields[#fields + 1] = "key" .. i .. "=value"..i end
    fields = table.concat(fields, '&')
    c = assert(curl.easy{
      url           = "http://httpbin.org/post",
      postfields    = fields,
      writefunction = function()end,
    })
    pfields = weak_ptr(fields)
  end

  -- call gc to try clear `fields` string
  for i = 1, 4 do collectgarbage"collect" end
  assert_string(pfields.value)

  assert_equal(c, c:unsetopt_postfields())

  -- @todo check internal storage because gc really do not clear `weak` string
  -- for i = 1, 4 do collectgarbage"collect" end
  -- assert_nil(pfields.value)

  -- c:perform()
end

end

local _ENV = TEST_CASE'setopt_user_data'     if ENABLE then

local c

function setup()
  if c then c:close() end
  c = nil
end

function test_data()
  c = assert(curl.easy())
  assert_nil(c:getdata())
  c:setdata("hello")
  assert_equal("hello", c:getdata())
end

function test_cleanup()
  local ptr do
    local t = {}
    local e = curl.easy():setdata(t)
    ptr = weak_ptr(t)
    gc_collect()

    assert_equal(t, ptr.value)
  end

  gc_collect()
  assert_nil(ptr.value)
end

end

RUN()
