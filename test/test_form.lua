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

local curl      = require "lcurl"

local _ENV = TEST_CASE'add_content' do

local post

function setup()
  post = curl.form()
end

function teardown()
  if post then post:free() end
  post = nil
end

local function F(...)
  local data = assert_string(post:get())
  post:free()
  post = nil
  return data
end

function test_01()
  assert_equal(post, post:add_content('name01', 'value01'))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue01\r\n", data)
  assert_match('name="name01"', data)
end

function test_02()
  assert_equal(post, post:add_content('name02', 'value02', "text/plain"))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue02\r\n", data)
  assert_match('name="name02"', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_03()
  assert_equal(post, post:add_content('name03', 'value03', {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue03\r\n", data)
  assert_match('name="name03"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_04()
  assert_equal(post, post:add_content('name04', 'value04', "text/plain", {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue04\r\n", data)
  assert_match('name="name04"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_error()
  assert_error(function() post:add_content()        end)
  assert_error(function() post:add_stream(nil)      end)
  assert_error(function() post:add_stream('name01') end)
end

end

local _ENV = TEST_CASE'add_buffer' do

local post

function setup()
  post = curl.form()
end

function teardown()
  if post then post:free() end
  post = nil
end

function test_01()
  assert_equal(post, post:add_buffer('name01', 'file01', 'value01'))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue01\r\n", data)
  assert_match('name="name01"', data)
  assert_match('filename="file01"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_02()
  assert_equal(post, post:add_buffer('name02', 'file02', 'value02', "text/plain"))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue02\r\n", data)
  assert_match('name="name02"', data)
  assert_match('filename="file02"', data)
  assert_match('Content%-Type: text/plain\r\n', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_03()
  assert_equal(post, post:add_buffer('name03', 'file03', 'value03', {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue03\r\n", data)
  assert_match('name="name03"', data)
  assert_match('filename="file03"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_04()
  assert_equal(post, post:add_buffer('name04', 'file04', 'value04', "text/plain", {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue04\r\n", data)
  assert_match('name="name04"', data)
  assert_match('filename="file04"', data)
  assert_match('Content%-Type: text/plain\r\n', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_05()
  assert_equal(post, post:add_buffer('name05', 'file05', 'value05', nil, {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  assert_match("\r\n\r\nvalue05\r\n", data)
  assert_match('name="name05"', data)
  assert_match('filename="file05"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_error()
  assert_error(function() post:add_buffer()         end)
  assert_error(function() post:add_buffer(nil)      end)
  assert_error(function() post:add_buffer('name01') end)
  assert_error(function() post:add_buffer('name02', 'file02') end)
  assert_error(function() post:add_buffer('name03', 'file03', nil) end)
  assert_error(function() post:add_buffer('name04', nil, 'value04') end)
end

end

local _ENV = TEST_CASE'add_stream' do

local post

local dummy = function()end

local stream = function() return 128, dummy end

function setup()
  post = curl.form()
end

function teardown()
  if post then post:free() end
  post = nil
end

function test_01()
  assert_equal(post, post:add_stream('name01', stream()))
  local data = assert_string(post:get())
  assert_match('name="name01"', data)
  assert_not_match('filename', data)
end

function test_02()
  assert_equal(post, post:add_stream('name02', 'file02', stream()))
  local data = assert_string(post:get())
  assert_match('name="name02"', data)
  assert_match('filename="file02"', data)
end

function test_03()
  assert_equal(post, post:add_stream('name03', 'file03', 'text/plain', stream()))
  local data = assert_string(post:get())
  assert_match('name="name03"', data)
  assert_match('filename="file03"', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_04()
  assert_equal(post, post:add_stream('name04', 'file04', 'text/plain', {"Content-Encoding: gzip"}, stream()))
  local data = assert_string(post:get())
  assert_match('name="name04"', data)
  assert_match('filename="file04"', data)
  assert_match('Content%-Type: text/plain\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_05()
  assert_equal(post, post:add_stream('name05', 'file05', {"Content-Encoding: gzip"}, stream()))
  local data = assert_string(post:get())
  assert_match('name="name05"', data)
  assert_match('filename="file05"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_06()
  assert_equal(post, post:add_stream('name06', {"Content-Encoding: gzip"}, stream()))
  local data = assert_string(post:get())
  assert_match('name="name06"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_07()
  assert_equal(post, post:add_stream('name07', 'file07', nil, {"Content-Encoding: gzip"}, stream()))
  local data = assert_string(post:get())
  assert_match('name="name07"', data)
  assert_match('filename="file07"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_08()
  assert_equal(post, post:add_stream('name08', nil, nil, {"Content-Encoding: gzip"}, stream()))
  local data = assert_string(post:get())
  assert_match('name="name08"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_09()
  assert_equal(post, post:add_stream('name09', nil, nil, nil, stream()))
  local data = assert_string(post:get())
  assert_match('name="name09"', data)
end

function test_error()
  assert_error(function() post:add_stream('name01', dummy) end)
  assert_error(function() post:add_stream('name02', 10)    end)
  assert_error(function() post:add_stream('name03', "10", dummy)    end)
end

function test_add_several()
  assert_equal(post, post:add_stream('name01', stream()))
  assert_equal(post, post:add_stream('name02', 'file02', stream()))
  local data = assert_string(post:get())
  assert_match('name="name01"', data)
  assert_match('name="name02"', data)
  assert_match('filename="file02"', data)
end

end

local _ENV = TEST_CASE'add_file' do

local post

local form_file  = "file.form"
local form_path  = "./" .. form_file
local form_data  = "some values"

local function mkfile(P, data)
  local f, e = io.open(P, "w+b")
  if not f then return nil, err end
  if data then assert(f:write(data)) end
  f:close()
  return P
end

local function check_form(data)
  assert_match('name="nameXX"', data)
  assert_match("\r\n\r\n" .. form_data .."\r\n", data)
end

function setup()
  post = curl.form()
  assert(mkfile(form_path, form_data))
end

function teardown()
  if post then post:free() end
  post = nil
  os.remove(form_file)
end

function test_01()
  assert_equal(post, post:add_file('nameXX', form_path))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_02()
  assert_equal(post, post:add_file('nameXX', form_path, 'text/plain'))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_03()
  assert_equal(post, post:add_file('nameXX', form_path, 'text/plain', 'renamedfile'))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. 'renamedfile' .. '"', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_04()
  assert_equal(post, post:add_file('nameXX', form_path, nil, 'renamedfile'))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. 'renamedfile' .. '"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
end

function test_05()
  assert_equal(post, post:add_file('nameXX', form_path, 'text/plain', 'renamedfile', {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. 'renamedfile' .. '"', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_05()
  assert_equal(post, post:add_file('nameXX', form_path, 'text/plain', {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_not_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_06()
  assert_equal(post, post:add_file('nameXX', form_path, {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_07()
  assert_equal(post, post:add_file('nameXX', form_path, nil, {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_08()
  assert_equal(post, post:add_file('nameXX', form_path, nil, nil, {"Content-Encoding: gzip"}))
  local data = assert_string(post:get())
  check_form(data)
  assert_match('filename="' .. form_file .. '"', data)
  assert_match('Content%-Type: application/octet%-stream\r\n', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
end

function test_error()
  assert_error(function()
    assert_equal(post, post:add_file('nameXX', 'text/plain', form_path))
    post:get()
  end)
end

end

local _ENV = TEST_CASE'get' do

local post

function setup()
  post = curl.form()
end

function teardown()
  if post then post:free() end
  post = nil
end

local function check_form(data)
  assert_match("\r\n\r\nvalueXX\r\n", data)
  assert_match('name="nameXX"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_writer_01()
  local t = {}
  local function writer(str, ...)
    assert_equal(0, select("#", ...))
    t[#t+1] = str
  end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  assert_equal(post, post:get(writer))
  check_form(table.concat(t))
end

function test_writer_02()
  local t = {}
  local function writer(str, ...)
    assert_equal(0, select("#", ...))
    t[#t+1] = str
    return true
  end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  assert_equal(post, post:get(writer))
  check_form(table.concat(t))
end

function test_writer_03()
  local t = {}
  local function writer(str, ...)
    assert_equal(0, select("#", ...))
    t[#t+1] = str
    return #str
  end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  assert_equal(post, post:get(writer))
  check_form(table.concat(t))
end

function test_writer_context()
  local t = {}
  local function writer(T, str, ...)
    assert_equal(t, T)
    assert_equal(0, select("#", ...))
    T[#T+1] = str
  end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  assert_equal(post, post:get(writer, t))
  local data = table.concat(t)
  assert_match("\r\n\r\nvalueXX\r\n", data)
  assert_match('name="nameXX"', data)
  assert_match('Content%-Encoding: gzip\r\n', data)
  assert_match('Content%-Type: text/plain\r\n', data)
end

function test_abort_01()
  local err = {}
  local function writer() return nil, err end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  local _, e = assert_nil(post:get(writer))
  assert_equal(err, e)
end

function test_abort_02()
  local function writer() return 0 end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  local _, e = assert_nil(post:get(writer))
  assert_nil(e)
end

function test_abort_03()
  local function writer() return nil end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  local _, e = assert_nil(post:get(writer))
  assert_nil(e)
end

function test_abort_04()
  local function writer() return false end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  local _, e = assert_nil(post:get(writer))
  assert_nil(e)
end

function test_error()
  local err = {}
  local function writer() error("WRITEERROR") end
  assert_equal(post, post:add_content('nameXX', 'valueXX', "text/plain", {"Content-Encoding: gzip"}))
  assert_error_match("WRITEERROR", function()
    post:get(writer)
  end)
end

end

RUN()
