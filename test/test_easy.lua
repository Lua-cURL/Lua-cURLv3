local HAS_RUNNER = not not lunit
local lunit      = require "lunit"
local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end

local curl       = require "lcurl"
local scurl      = require "lcurl.safe"
local url        = "http://example.com"
local fname      = "./test.download"

local function weak_ptr(val)
  return setmetatable({value = val},{__mode = 'v'})
end

local function gc_collect()
  collectgarbage("collect")
  collectgarbage("collect")
end

local _ENV = TEST_CASE'write_callback' do

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

local _ENV = TEST_CASE'progress_callback' do

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

local _ENV = TEST_CASE'escape' do

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

local _ENV = TEST_CASE'setopt_form' do

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

if not HAS_RUNNER then lunit.run() end
