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
local utils      = require "utils"
local fname      = "./test.download"

-- local GET_URL  = "http://example.com"
-- local POST_URL = "http://httpbin.org/post"
local GET_URL  = "http://127.0.0.1:7090/get"
local POST_URL = "http://127.0.0.1:7090/post"

-- print("------------------------------------")
-- print("Lua  version: " .. (_G.jit and _G.jit.version or _G._VERSION))
-- print("cURL version: " .. curl.version())
-- print("------------------------------------")
-- print("")

local weak_ptr, gc_collect, is_curl_ge, read_file, stream, Stream, dump_request =
  utils.import('weak_ptr', 'gc_collect', 'is_curl_ge', 'read_file', 'stream', 'Stream', 'dump_request')

local null = curl.null

local ENABLE = true

local _ENV = TEST_CASE'curl error'           if ENABLE then

function test_eq_with_same_cat()
  local e1 = curl.error(curl.ERROR_EASY, curl.E_OK)
  local e2 = curl.error(curl.ERROR_EASY, curl.E_OK)
  assert_equal(e1, e2)
end

function test_eq_with_different_cat()
  local e1 = curl.error(curl.ERROR_EASY, curl.E_OK)
  local e2 = curl.error(curl.ERROR_FORM, curl.E_OK)

  assert_equal(e1:no(), e2:no())
  assert_not_equal(e1, e2)
end

function test_ctor_cat()
  local e

  e = curl.error(curl.ERROR_EASY, curl.E_OK)
  assert_equal(e:category(), curl.ERROR_EASY)
  assert_equal(e:no(), curl.E_OK)

  e = curl.error(curl.ERROR_MULTI, curl.E_OK)
  assert_equal(e:category(), curl.ERROR_MULTI)
  assert_equal(e:no(), curl.E_OK)

  e = curl.error(curl.ERROR_SHARE, curl.E_OK)
  assert_equal(e:category(), curl.ERROR_SHARE)
  assert_equal(e:no(), curl.E_OK)

  e = curl.error(curl.ERROR_FORM, curl.E_OK)
  assert_equal(e:category(), curl.ERROR_FORM)
  assert_equal(e:no(), curl.E_OK)

  assert_error(function()
    curl.error(nil, curl.E_OK)
  end)

  assert_error(function()
    curl.error('UNKNOWN STRING', curl.E_OK)
  end)

end

end

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
    url = GET_URL;
    writefunction = f;
  })

  assert_equal(c, c:perform())
end

function test_write_abort_01()
  c = assert(scurl.easy{
    url = GET_URL;
    writefunction = function(str) return #str - 1 end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_write_abort_02()
  c = assert(scurl.easy{
    url = GET_URL;
    writefunction = function(str) return false end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_write_abort_03()
  c = assert(scurl.easy{
    url = GET_URL;
    writefunction = function(str) return nil, "WRITEERROR" end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal("WRITEERROR", e)
end

function test_write_abort_04()
  c = assert(scurl.easy{
    url = GET_URL;
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
  assert_equal(c, c:setopt_writefunction(print, null))
  assert_equal(c, c:setopt_writefunction(null))
  assert_equal(c, c:setopt_writefunction(null, nil))
  assert_equal(c, c:setopt_writefunction(null, null))
  assert_error(function()c:setopt_writefunction()end)
  assert_error(function()c:setopt_writefunction(nil)end)
  assert_error(function()c:setopt_writefunction(nil, f)end)
  assert_error(function()c:setopt_writefunction(null, {})end)
  assert_error(function()c:setopt_writefunction(print, {}, nil)end)
  assert_error(function()c:setopt_writefunction(print, {}, null)end)
end

function test_write_pass_01()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = function(s) return #s end
  })

  assert_equal(c, c:perform())
end

function test_write_pass_02()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = function() return  end
  })

  assert_equal(c, c:perform())
end

function test_write_pass_03()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = function() return true end
  })

  assert_equal(c, c:perform())
end

function test_write_coro()
  local co1, co2
  local called

  co1 = coroutine.create(function()
    c = assert(curl.easy{
      url = GET_URL;
      writefunction = function()
        called = coroutine.running()
        return true
      end
    })
    coroutine.yield()
  end)

  co2 = coroutine.create(function()
    assert_equal(c, c:perform())
  end)

  coroutine.resume(co1)
  coroutine.resume(co2)

  assert_equal(co2, called)
end

function test_write_pass_null_context()
  c = assert(curl.easy{
    url = GET_URL;
  })

  local context
  assert_equal(c, c:setopt_writefunction(function(ctx)
    context = ctx
    return true
  end, null))

  assert_equal(c, c:perform())
  assert_equal(null, context)
end

function test_write_pass_nil_context()
  c = assert(curl.easy{
    url = GET_URL;
  })

  local context, called
  assert_equal(c, c:setopt_writefunction(function(ctx)
    context = ctx
    called = true
    return true
  end, nil))

  assert_equal(c, c:perform())
  assert_true(called)
  assert_nil(context)
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
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return false end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_02()
  c  = assert(scurl.easy{
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return 0 end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_03()
  c  = assert(scurl.easy{
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return nil end
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), e)
end

function test_abort_04()
  c  = assert(scurl.easy{
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return nil, "PROGRESSERROR" end
  })

  local _, e = assert_nil(c:perform())
  assert_equal("PROGRESSERROR", e)
end

function test_abort_05()
  c  = assert(scurl.easy{
    url              = GET_URL,
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
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() end
  })

  assert_equal(c, c:perform())
end

function test_pass_02()
  c  = assert(scurl.easy{
    url              = GET_URL,
    writefunction    = pass,
    noprogress       = false,
    progressfunction = function() return true end
  })

  assert_equal(c, c:perform())
end

function test_pass_03()
  c  = assert(scurl.easy{
    url              = GET_URL,
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
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function(str) return #str - 1 end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_header_abort_02()
  c = assert(scurl.easy{
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function(str) return false end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal(curl.error(curl.ERROR_EASY, curl.E_WRITE_ERROR), e)
end

function test_header_abort_03()
  c = assert(scurl.easy{
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function(str) return nil, "WRITEERROR" end;
  })

  local _, e = assert_nil(c:perform())
  assert_equal("WRITEERROR", e)
end

function test_header_abort_04()
  c = assert(scurl.easy{
    url = GET_URL;
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
  assert_equal(c, c:setopt_headerfunction(null))
  assert_equal(c, c:setopt_headerfunction(null, nil))
  assert_equal(c, c:setopt_headerfunction(null, null))
  assert_error(function()c:setopt_headerfunction()end)
  assert_error(function()c:setopt_headerfunction(nil)end)
  assert_error(function()c:setopt_headerfunction(nil, f)end)
  assert_error(function()c:setopt_headerfunction(null, {})end)
  assert_error(function()c:setopt_headerfunction(print, {}, nil)end)
end

function test_header_pass_01()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function(s) return #s end
  })

  assert_equal(c, c:perform())
end

function test_header_pass_02()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function() return  end
  })

  assert_equal(c, c:perform())
end

function test_header_pass_03()
  c = assert(curl.easy{
    url = GET_URL;
    writefunction = dummy,
    headerfunction = function() return true end
  })

  assert_equal(c, c:perform())
end


end

local _ENV = TEST_CASE'read_stream_callback' if ENABLE and is_curl_ge(7,30,0) then

-- tested on WinXP(x32)/Win8(x64) libcurl/7.37.1 / libcurl/7.30.0

local url = POST_URL

local m, c, f, t

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
  if m then m:close() end
  t, f, c, m = nil
end

function test()
  assert_equal(f, f:add_stream('SSSSS', stream('X', 128, 13)))
  assert_equal(c, c:setopt_httppost(f))

  -- should be called only stream callback
  local read_called
  assert_equal(c, c:setopt_readfunction(function()
    read_called = true
  end))

  assert_equal(c, c:perform())

  assert_nil(read_called)

  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())
  assert_table(data.form)
  assert_equal(('X'):rep(128), data.form.SSSSS)
end

function test_object()
  local s = Stream('X', 128, 13)

  assert_equal(f, f:add_stream('SSSSS', s:size(), s))
  assert_equal(c, c:setopt_httppost(f))
  assert_equal(c, c:perform())

  assert_equal(s, s.called_ctx)

  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())
  assert_table(data.form)
  assert_equal(('X'):rep(128), data.form.SSSSS)
end

function test_co_multi()
  local s = Stream('X', 128, 13)
  assert_equal(f, f:add_stream('SSSSS', s:size(), s))
  assert_equal(c, c:setopt_httppost(f))

  m = assert(scurl.multi())
  assert_equal(m, m:add_handle(c))

  co = coroutine.create(function()
    while 1== m:perform() do end
  end)

  coroutine.resume(co)

  assert_equal(co, s.called_co)

  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())
  assert_table(data.form)
  assert_equal(('X'):rep(128), data.form.SSSSS)
end

function test_co()
  local s = Stream('X', 128, 13)

  assert_equal(f, f:add_stream('SSSSS', s:size(), s))
  assert_equal(c, c:setopt_httppost(f))

  co = coroutine.create(function()
    assert_equal(c, c:perform())
  end)

  coroutine.resume(co)

  assert_equal(co, s.called_co)

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

local uname = upath:normalize(path.fullpath(fname))

local url   = "FILE:///" .. uname

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

  assert_equal(e, e:unsetopt_httppost())

  gc_collect()
  assert(not pfrom.value)
end

function test_reset()
  local pfrom, e
  do
    local form = curl.form()
    e = curl.easy{httppost = form}
    pform = weak_ptr(form)
  end

  gc_collect()
  assert(pform.value)

  assert_equal(e, e:reset())

  gc_collect()
  assert(not pform.value)
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
    for i = 1, 100 do fields[#fields + 1] = string.format('key%d=value%d', i, i) end
    fields = table.concat(fields, '&')
    c = assert(curl.easy{
      url           = POST_URL,
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
    for i = 1, 100 do fields[#fields + 1] = string.format('key%d=value%d', i, i) end
    fields = table.concat(fields, '&')
    c = assert(curl.easy{
      url           = POST_URL,
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

function teardown()
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

local _ENV = TEST_CASE'multi_add_remove'     if ENABLE then

local m, c

function setup()
  m = assert(scurl.multi())
end

function teardown()
  if c then c:close() end
  if m then m:close() end
  m, c = nil
end

function test_remove_unknow_easy()
  c = assert(scurl.easy())
  assert_equal(m, m:remove_handle(c))
end

function test_double_remove_easy()
  c = assert(scurl.easy())
  assert_equal(m, m:add_handle(c))
  assert_equal(m, m:remove_handle(c))
  assert_equal(m, m:remove_handle(c))
end

function test_double_add_easy()
  c = assert(scurl.easy())
  assert_equal(m, m:add_handle(c))
  assert_nil(m:add_handle(c))
end

end

local _ENV = TEST_CASE'unset_callback_ctx'   if ENABLE then

local HSTS = curl.version_info().features.HSTS

local c

function setup()
  c = assert(scurl.easy())
end

function teardown()
  if c then c:close() end
  c = nil
end

local function test_cb(name)
  local set, unset = 'setopt_' .. name, 'unsetopt_' .. name

  set   = assert_function(c[set],   set)
  unset = assert_function(c[unset], unset)

  local pctx
  do local ctx = {}
    pctx = weak_ptr(ctx)
    assert(set(c, function() end, ctx))
  end

  gc_collect()
  assert_table(pctx.value)

  unset(c)

  gc_collect()
  assert_nil(pctx.value)

  do local ctx = {}
    pctx = weak_ptr(ctx)
    assert(set(c, function() end, ctx))
  end

  gc_collect()
  assert_table(pctx.value)

  c:reset()

  gc_collect()
  assert_nil(pctx.value)
end

function test_read()      test_cb('readfunction')       end
function test_write()     test_cb('writefunction')      end
function test_header()    test_cb('headerfunction')     end
function test_progress()  test_cb('progressfunction')   end
function test_seek()      test_cb('seekfunction')       end
function test_debug()     test_cb('debugfunction')      end
function test_fnmatch()   test_cb('fnmatch_function')   end
function test_chunk_bgn() test_cb('chunk_bgn_function') end
function test_chunk_end() test_cb('chunk_end_function') end

if curl.OPT_HSTSREADFUNCTION and HSTS then
function test_hstsreadfunction()  test_cb('hstsreadfunction')  end
function test_hstswritefunction() test_cb('hstswritefunction') end
end

end

local _ENV = TEST_CASE'set_slist'            if ENABLE then

local c

function teardown()
  if c then c:close() end
  c = nil
end

function test_set()
  c = curl.easy()
  c:setopt_httpheader({'X-Custom: value'})
  local body, headers = assert_string(dump_request(c))
  assert_match("X%-Custom:%s*value\r\n", headers)
end

function test_unset()
  c = curl.easy()
  c:setopt_httpheader({'X-Custom: value'})
  c:unsetopt_httpheader()
  local body, headers = assert_string(dump_request(c))
  assert_not_match("X%-Custom:%s*value\r\n", headers)
end

function test_set_empty_array()
  c = curl.easy()
  c:setopt_httpheader({'X-Custom: value'})
  c:setopt_httpheader({})
  local body, headers = assert_string(dump_request(c))
  assert_not_match("X%-Custom:%s*value\r\n", headers)
end

function test_reset_slist()
  c = curl.easy {
    httpheader = {'X-Foo: 1'},
    resolve    = {'example.com:80:127.0.0.1'}
  }

  c:reset()

  c:setopt{
    httpheader = {'X-Foo: 2'},
    resolve    = {'example.com:80:127.0.0.1'}
  }

  local body, headers = assert_string(dump_request(c))
  assert_match("X%-Foo:%s2\r\n", headers)
end

end

local _ENV = TEST_CASE'set_null'             if ENABLE then

local c, m

function teardown()
  if c then c:close() end
  if m then m:close() end
  m, c = nil
end

function test_string()
  c = curl.easy()
  c:setopt_accept_encoding('gzip')
  local body, headers = assert_string(dump_request(c))
  assert_match("Accept%-Encoding:%s*gzip", headers)

  c:setopt_accept_encoding(null)
  body, headers = assert_string(dump_request(c))
  assert_not_match("Accept%-Encoding:%s*gzip", headers)
end

function test_string_via_table()
  c = curl.easy()
  c:setopt_accept_encoding('gzip')
  local body, headers = assert_string(dump_request(c))
  assert_match("Accept%-Encoding:%s*gzip", headers)

  c:setopt{ accept_encoding = null }
  body, headers = assert_string(dump_request(c))
  assert_not_match("Accept%-Encoding:%s*gzip", headers)
end

function test_slist()
  c = curl.easy()
  c:setopt_httpheader({'X-Custom: value'})
  c:setopt_httpheader(null)
  local body, headers = assert_string(dump_request(c))
  assert_not_match("X%-Custom:%s*value\r\n", headers)
end

function test_slist_via_table()
  c = curl.easy()
  c:setopt_httpheader({'X-Custom: value'})
  c:setopt{httpheader = null}
  local body, headers = assert_string(dump_request(c))
  assert_not_match("X%-Custom:%s*value\r\n", headers)
end

function test_multi_set_array()
  m = curl.multi()
  m:setopt_pipelining_site_bl{
    '127.0.0.1'
  }
  assert_equal(m, m:setopt_pipelining_site_bl(null))
end

end

local _ENV = TEST_CASE'trailer_callback'     if ENABLE and is_curl_ge(7,64,0) then

local url = POST_URL

local m, c, t

local function json_data()
  return json.decode(table.concat(t))
end

local treader = function(t)
  local i = 0
  return function()
    i = i + 1
    return t[i]
  end
end

function setup()
  t = {}
  c = assert(scurl.easy{
    url           = url,
    post          = true,
    httpheader    = {"Transfer-Encoding: chunked"},
    readfunction  = treader {'a=1&', 'b=2&'},
    timeout       = 60,
  })
  assert_equal(c, c:setopt_writefunction(table.insert, t))
end

function teardown()
  if c then c:close() end
  if m then m:close() end
  t, c, m = nil
end

local empty_responses = {
  {'no_response',   function()                  end},
  {'nil_response',  function() return nil       end},
  {'null_response', function() return curl.null end},
  {'true_response', function() return true      end},
  {'empty_array',   function() return {}        end},
}

local abort_responses = {
  {'false_response',          function() return false end},
  {'nil_with_error_response', function() return nil, 'error message' end},
  {'numeric_response_0',      function() return 0 end},
  {'numeric_response_1',      function() return 1 end},
}

for _, response in ipairs(empty_responses) do
  _ENV[ 'test_' .. response[1] ] = function()
    local trailer_called = 0
    assert_equal(c, c:setopt_trailerfunction(function()
      trailer_called = trailer_called + 1
      return response[2]()
    end))

    assert_equal(c, c:perform())

    assert_equal(1, trailer_called)

    assert_equal(200, c:getinfo_response_code())
    local data = assert_table(json_data())

    assert_equal('1', data.form.a)
    assert_equal('2', data.form.b)
  end
end

for _, response in ipairs(abort_responses) do
  _ENV[ 'test_' .. response[1] ] = function()
    local trailer_called = 0
    assert_equal(c, c:setopt_trailerfunction(function()
      trailer_called = trailer_called + 1
      return response[2]()
    end))

    local ok, err = assert_nil(c:perform())
    assert_equal(1, trailer_called)
    assert_equal(curl.error(curl.ERROR_EASY, curl.E_ABORTED_BY_CALLBACK), err)
  end
end

function test_send_header()
  local trailer_called = 0
  assert_equal(c, c:setopt_trailerfunction(function()
    trailer_called = trailer_called + 1
    return {'x-trailer-header: value'}
  end))

  assert_equal(c, c:perform())

  assert_equal(1, trailer_called)

  assert_equal(200, c:getinfo_response_code())
  local data = assert_table(json_data())

  assert_equal('1', data.form.a)
  assert_equal('2', data.form.b)
  assert_equal('value', data.headers['x-trailer-header'])
end

end

RUN()
