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

local _,luacov   = pcall(require, "luacov")
local TEST_CASE  = assert(lunit.TEST_CASE)
local skip       = lunit.skip or function() end

local curl       = require "cURL"
local scurl      = require "cURL.safe"
local json       = require "dkjson"
local fname      = "./test.download"

local utils = require "utils"

local weak_ptr, gc_collect, is_curl_ge, is_curl_eq, read_file, stream, Stream, dump_request =
  utils.import('weak_ptr', 'gc_collect', 'is_curl_ge', 'is_curl_eq', 'read_file', 'stream', 'Stream', 'dump_request')

-- Bug. libcurl 7.56.0 does not add `Content-Type: text/plain`
local text_plain = is_curl_eq(7,56,0) and 'test/plain' or 'text/plain'

local GET_URL = "http://127.0.0.1:7090/get"

local ENABLE = true

local _ENV = TEST_CASE'version'        if ENABLE then

function test()
  assert_match("^%d+%.%d+%.%d+%-?", curl._VERSION)
  assert_equal("Lua-cURL",          curl._NAME   )
end

end

local _ENV = TEST_CASE'easy'           if ENABLE then

local e1, e2
function teardown()
  if e1 then e1:close() end
  if e2 then e2:close() end
  e1, e2 = nil
end

if curl.OPT_STREAM_DEPENDS then

function test_easy_setopt_stream_depends_1()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt_stream_depends(e2)
  end)
end

function test_easy_setopt_stream_depends_2()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt(curl.OPT_STREAM_DEPENDS, e2)
  end)
end

function test_easy_setopt_stream_depends_3()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt{[curl.OPT_STREAM_DEPENDS] = e2}
  end)
end

function test_easy_setopt_stream_depends_4()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt{stream_depends = e2}
  end)
end

function test_easy_setopt_stream_depends_e_1()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt_stream_depends_e(e2)
  end)
end

function test_easy_setopt_stream_depends_e_2()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt(curl.OPT_STREAM_DEPENDS_E, e2)
  end)
end

function test_easy_setopt_stream_depends_e_3()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt{[curl.OPT_STREAM_DEPENDS_E] = e2}
  end)
end

function test_easy_setopt_stream_depends_e_4()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.easy())
  assert_pass(function()
    e1:setopt{stream_depends_e = e2}
  end)
end

end

function test_easy_setopt_share()
  e1 = assert(scurl.easy())
  e2 = assert(scurl.share())
  assert_pass(function()
    e1:setopt_share(e2)
  end)
end

end

local _ENV = TEST_CASE'multi_iterator' if ENABLE then

local url = GET_URL

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
  local base_url = url .. '?key='
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

  assert_equal(m, m:add_handle(next_easy()))

  for data, type, easy in m:iperform() do

    if type == "done" or type == "error" then
      assert_equal(urls[i], easy:getinfo_effective_url())
      assert_equal(easy, c)
      easy:close()
      c = nil

      if i == 3 then
        if is_curl_ge(7, 62,0) then
          assert_equal(curl.error(curl.ERROR_EASY, curl.E_URL_MALFORMAT), data)
        else
          assert_equal(curl.error(curl.ERROR_EASY, curl.E_UNSUPPORTED_PROTOCOL), data)
        end
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

function test_info_read()

  local url = GET_URL .. '?key=1'

  c = assert(curl.easy{url=url, writefunction=function() end})
  assert_equal(m, m:add_handle(c))

  while m:perform() > 0 do m:wait() end

  local h, ok, err = m:info_read()
  assert_equal(c, h)

  local h, ok, err = m:info_read()
  assert_equal(0, h)
end

end

local _ENV = TEST_CASE'multi memory leak' if ENABLE then

local m

function teardown()
  if m then m:close() end
  m = nil
end

function test_basic()
  local ptr do 
    local multi = assert(scurl.multi())
    ptr = weak_ptr(multi)
  end
  gc_collect()

  assert_nil(ptr.value)
end

function test_socket_action()
  local ptr do 
    local multi = assert(scurl.multi())
    multi:setopt_socketfunction(function() end)
    ptr = weak_ptr(multi)
  end
  gc_collect()

  assert_nil(ptr.value)
end

end

local _ENV = TEST_CASE'form'           if ENABLE then

local post

function teardown()
  if post then post:free() end
  post = nil
end

function test_content_01()
  post = assert(scurl.form{name01 = 'value01'})
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue01\r\n", data)
  assert_match('name="name01"', data)
end

function test_content_02()
  post = assert(scurl.form{name02 = {'value02', type = text_plain}})
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue02\r\n", data)
  assert_match('name="name02"', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
end

function test_content_03()
  post = assert(scurl.form{name03 = {content = 'value03', headers = {"Content-Encoding: gzip"}}})
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue03\r\n", data)
  assert_match('name="name03"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_content_04()
  post = assert(scurl.form{name04 = {'value04', type = text_plain, headers = {"Content-Encoding: gzip"}}})
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue04\r\n", data)
  assert_match('name="name04"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
end

function test_buffer_01()
  post = assert(scurl.form{name01 = {
    name = 'file01',
    data = 'value01',
  }})

  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue01\r\n", data)
  assert_match('name="name01"', data)
  assert_match('filename="file01"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_buffer_02()
  post = assert(scurl.form{name02 = {
    name = 'file02',
    data = 'value02',
    type = text_plain,
  }})

  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue02\r\n", data)
  assert_match('name="name02"', data)
  assert_match('filename="file02"', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_buffer_03()
  post = assert(scurl.form{name03 = {
    name = 'file03',
    data = 'value03',
    headers = {"Content-Encoding: gzip"},
  }})
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue03\r\n", data)
  assert_match('name="name03"', data)
  assert_match('filename="file03"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_buffer_04()
  post = assert(scurl.form{name04 = {
    name = 'file04',
    data = 'value04',
    type = text_plain,
    headers = {"Content-Encoding: gzip"},
  }})

  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue04\r\n", data)
  assert_match('name="name04"', data)
  assert_match('filename="file04"', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_stream_01()
  post = assert(scurl.form{name01 = {
    stream = function() end,
    length = 128,
  }})
  local data = assert_string(post:get())
  assert_match('name="name01"', data)
  assert_not_match('filename', data)
end

function test_stream_02()
  post = assert(scurl.form{name02 = {
    name   = 'file02',
    stream = function() end,
    length = 128,
  }})
  local data = assert_string(post:get())
  assert_match('name="name02"', data)
  assert_match('filename="file02"', data)
end

function test_stream_03()
  post = assert(scurl.form{name03 = {
    name   = 'file03',
    stream = function() end,
    length = 128,
    type   = text_plain,
  }})

  local data = assert_string(post:get())
  assert_match('name="name03"', data)
  assert_match('filename="file03"', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
end

function test_stream_04()
  post = assert(scurl.form{name04 = {
    name   = 'file04',
    stream = function() end,
    length = 128,
    type   = text_plain,
    headers = {"Content-Encoding: gzip"},
  }})
  local data = assert_string(post:get())
  assert_match('name="name04"', data)
  assert_match('filename="file04"', data)
  assert_match('Content%-Type: ' .. text_plain .. '\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_stream_05()
  post = assert(scurl.form{name05 = {
    stream = {
      length = function() return 128 end;
      read   = function() end;
    }
  }})
  local data = assert_string(post:get())
  assert_match('name="name05"', data)
  assert_not_match('filename', data)
end

function test_error()
  assert_error(function() post = scurl.form{name = {content = 1}}             end)
  assert_error(function() post = scurl.form{name = {1}}                       end)
  assert_error(function() post = scurl.form{name = {data = {}}}               end)
  assert_error(function() post = scurl.form{name = {file = true}}             end)
  assert_error(function() post = scurl.form{name = {stream = function() end}} end)
  assert_error(function() post = scurl.form{name = {stream = {}}}             end)
  assert_error(function() post = scurl.form{name = {stream = {
    read=function()end;length=function()end
  }}}end)
  assert_error(function() post = scurl.form{name = {stream = {
    read=function()end;length=function() return "123" end
  }}}end)
  assert_error(function() post = scurl.form{name = {stream = {
    read=function()end;length=function() return "hello" end
  }}}end)
end

function test_ignore_unknown()
  post = assert(scurl.form{
    name01 = {},
    name02 = {name = "helo"},
  })
  local data = assert_string(post:get())
  assert_not_match('name="name01"', data)
  assert_not_match('name="name02"', data)
end

function test_empty()
  post = assert(scurl.form{})
end

end

RUN()
