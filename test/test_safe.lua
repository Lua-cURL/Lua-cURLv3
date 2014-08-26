local HAS_RUNNER = not not lunit
local lunit      = require "lunit"
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

local c

function setup()
  c = assert(require"lcurl.safe".easy())
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

end

if not HAS_RUNNER then lunit.run() end
