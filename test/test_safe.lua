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

local _ENV = TEST_CASE'error_mode' do

local c

function teardown()
  if c then c:close() end
  c = nil
end

function test_safe()
  local curl = require "lcurl.safe"
  c = assert(curl.easy())
  assert_equal(c, c:setopt_url("aaaaa://123"))
  assert_nil(c:perform())
end

function test_raise()
  local curl = require "lcurl"
  c = assert(curl.easy())
  assert_equal(c, c:setopt_url("aaaaa://123"))
  assert_error(function() c:perform() end)
end

end

local _ENV = TEST_CASE'setopt' do

local curl = require "lcurl.safe"
local c

function setup()
  c = assert(curl.easy())
end

function teardown()
  if c then c:close() end
  c = nil
end

function test_number()
  assert_equal(c, c:setopt_verbose(false))
  assert_equal(c, c:setopt_verbose(true))
  assert_equal(c, c:setopt_verbose(1))
  assert_equal(c, c:setopt_verbose(0))
  assert_error(function() c:setopt_verbose("1") end)
  assert_error(function() c:setopt_verbose("true") end)
end

function test_string()
  assert_error(function() c:setopt_url(true) end)
  assert_error(function() c:setopt_url(1)    end)
  assert_equal(c, c:setopt_url("1"))
end

function test_array()
  assert_error(function() c:setopt_httpheader(true) end)
  assert_error(function() c:setopt_httpheader(1)    end)
  assert_error(function() c:setopt_httpheader("k:v")end)
  assert_equal(c, c:setopt_httpheader{"k:v"})
end

function test_multiple_options()
  assert_error(function() c:setopt{verbose = "false"} end)
  assert_error(function() c:setopt{verbose = "1"} end)
  assert_equal(c, c:setopt{verbose = false})
  assert_equal(c, c:setopt{[curl.OPT_VERBOSE] = false})
end

end

local _ENV = TEST_CASE'error_object' do

local curl = require "lcurl"

function test()
  local e1 = curl.error(curl.ERROR_EASY, 0) -- ok
  assert_equal(curl.ERROR_EASY, e1:category())
  assert_equal(curl.E_OK,       e1:no())
  assert_equal("OK",            e1:name())

  local e2 = curl.error(curl.ERROR_MULTI, 0) -- ok
  local e3 = curl.error(curl.ERROR_MULTI, 0) -- ok
  assert_equal(0, e1:no())
  assert_equal(0, e2:no())
  assert(e1 ~= e2)
  assert(e3 == e2)
end

end

local _ENV = TEST_CASE'ctor' do

local scurl = require "lcurl.safe"
local curl  = require "lcurl"
local c

function teardown()
  if c then c:close() end
  c = nil
end

function test_easy_error()
  c = assert(curl.easy())
  c:close()
  c = assert(curl.easy{
    url = "http://example.com",
    [curl.OPT_VERBOSE] = true,
  })
  c:close()
  
  assert_error(function()
      c = curl.easy{
        url_111 = "http://example.com",
    }
  end)
  
  assert_error(function()
      c = curl.easy{
        url = 123,
    }
  end)
end

function test_easy_safe()
  c = assert(scurl.easy())
  c:close()
  c = assert(scurl.easy{
    url = "http://example.com",
    [curl.OPT_VERBOSE] = true,
  })
  c:close()
  
  assert_pass(function()
      c = scurl.easy{
        url_111 = "http://example.com",
    }
  end)
  assert_nil(c)

  assert_error(function()
      c = scurl.easy{
        url = 123,
    }
  end)
end

function test_multi_error()
  c = assert(curl.multi())
  c:close()
  c = assert(curl.multi{
    maxconnects = 10;
    [curl.OPT_MULTI_PIPELINING] = true,
  })
  c:close()
  
  assert_error(function()
      c = curl.multi{
        url_111 = "http://example.com",
    }
  end)
  
  assert_error(function()
      c = curl.multi{
        maxconnects = "hello",
    }
  end)
end

function test_multi_safe()
  c = assert(scurl.multi())
  c:close()
  c = assert(scurl.multi{
    maxconnects = 10;
    [curl.OPT_MULTI_PIPELINING] = true,
  })
  c:close()
  
  assert_pass(function()
      c = scurl.multi{
        url_111 = "http://example.com",
    }
  end)
  assert_nil(c)

  assert_error(function()
      c = scurl.multi{
        maxconnects = "hello",
    }
  end)
end

function test_share_error()
  assert(curl.LOCK_DATA_COOKIE)

  c = assert(curl.share())
  c:close()
  c = assert(curl.share{
    share = curl.LOCK_DATA_COOKIE;
  })
  c:close()
  
  assert_error(function()
      c = curl.share{
        url_111 = "http://example.com",
    }
  end)
  
  assert_error(function()
      c = curl.share{
        share = "hello";
    }
  end)
end

function test_share_safe()
  assert(curl.LOCK_DATA_COOKIE)

  c = assert(scurl.share())
  c:close()
  c = assert(curl.share{
    share = scurl.LOCK_DATA_COOKIE;
  })
  c:close()
  
  assert_pass(function()
      c = scurl.share{
        url_111 = "http://example.com",
    }
  end)
  
  assert_error(function()
      c = scurl.share{
        share = "hello";
    }
  end)
end

end

local _ENV = TEST_CASE'objects_have_same_metatables' do

local scurl = require "lcurl.safe"
local curl  = require "lcurl"
local e1, e2, m

function teardown()
  if m  then m:close()  end
  if e1 then e1:close() end
  if e2 then e2:close() end
  e1, e2, m = nil
end

function test_1()
  e1 = assert(scurl.easy())
  e2 = assert(curl.easy())
  m  = assert(scurl.multi())

  assert_equal(m, m:add_handle(e1))
  assert_equal(m, m:add_handle(e2))
end

function test_2()
  e1 = assert(scurl.easy())
  e2 = assert(curl.easy())
  m  = assert(curl.multi())

  assert_equal(m, m:add_handle(e1))
  assert_equal(m, m:add_handle(e2))
end

end

RUN()
