local _,luacov = pcall(require, "luacov")

local HAS_RUNNER = not not lunit
local lunit      = require "lunit"
local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end

local curl       = require "cURL"
local scurl      = require "cURL.safe"
local json       = require "dkjson"
local fname      = "./test.download"

local ENABLE = true

local _ENV = TEST_CASE'multi_iterator' if ENABLE then

local url = "http://httpbin.org/get"

local c, t, m

local function json_data()
  return json.decode(table.concat(t))
end

function setup()
  t = {}
  m = assert(scurl.multi())
end

function teardown()
  if m then m:close() end
  if c then c:close() end
  m, c, t = nil
end

function test_add_handle()

  local base_url = 'http://httpbin.org/get?key='
  local urls = {
    base_url .. "1",
    base_url .. "2",
    "###" .. base_url .. "3",
    base_url .. "4",
    base_url .. "5",
  }

  local i = 0
  local function next_easy()
    i = i + 1
    local url = urls[i]
    if url then 
      c = assert(scurl.easy{url = url})
      t = {}
      return c
    end
  end

  m = assert_equal(m, m:add_handle(next_easy()))

  for data, type, easy in m:iperform() do

    if type == "done" or type == "error" then
      assert_equal(urls[i], easy:getinfo_effective_url())
      assert_equal(easy, c)
      easy:close()
      c = nil

      if i == 3 then
        assert_equal(curl.error(curl.ERROR_EASY, curl.E_UNSUPPORTED_PROTOCOL), data)
      else
        local data = json_data()
        assert_table(data.args)
        assert_equal(tostring(i), data.args.key)
      end

      easy = next_easy()
      if easy then m:add_handle(easy) end
    end

    if type == "data" then table.insert(t, data) end

  end

  assert_equal(#urls + 1, i)
  assert_nil(c)
end

end


if not HAS_RUNNER then lunit.run() end
