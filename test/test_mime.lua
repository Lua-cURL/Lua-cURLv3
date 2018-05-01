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
local utils     = require "utils"

local weak_ptr, gc_collect, dump_mime_ = utils.import('weak_ptr', 'gc_collect', 'dump_mime')

local GET_URL = 'http://127.0.0.1:7090/get'

local null = curl.null

local function is_freed(c)
  return not not string.find(tostring(c), '%(freed%)')
end

local _ENV = TEST_CASE'mime lifetime' if not curl.OPT_MIMEPOST then
function test() skip("MIMI API supports since cURL 7.56.0") end
else

local easy, mime

function setup()
  easy = curl.easy()
end

function teardown()
  if easy then easy:close() end
  if mime then mime:free() end
  easy, mime = nil
end

function test_preserve_mime_part_reference()
  -- mime part stores references to all parts

  local mime, part = easy:mime() do
    part = weak_ptr(mime:addpart())
  end
  gc_collect()

  assert_not_nil(part.value)

  mime = nil
  gc_collect()

  assert_nil(part.value)

  easy:close()
end

function test_free_mime_subparts()
  -- when free root free all nodes

  -- mime
  --  +- part3
  --      +- alt
  --          +- part1
  --          +- part2

  local mime, a, p1, p2, p3 = easy:mime() do

    local alt = easy:mime()

    local part1 = alt:addpart()

    local part2 = alt:addpart()

    local part3 = mime:addpart()
    part3:subparts(alt, "multipart/alternative")

    a  = weak_ptr(alt)
    p1 = weak_ptr(part1)
    p2 = weak_ptr(part2)
    p3 = weak_ptr(part3)
  end

  gc_collect()

  assert_not_nil(a.value)
  assert_not_nil(p1.value)
  assert_not_nil(p2.value)
  assert_not_nil(p3.value)

  -- reamove reference to root node
  mime = nil
  gc_collect(4)

  assert_nil(a.value)
  assert_nil(p1.value)
  assert_nil(p2.value)
  assert_nil(p3.value)

  easy:close()
end

function test_preserve_mime_subparts()
  -- if we have references to subnode but we free root
  -- then all references have to become to invalid

  -- mime
  --  +- part3
  --      +- alt
  --          +- part1
  --          +- part2

  local easy = curl.easy()

  local mime, a, p1, p2, p3 = easy:mime() do

    local alt = easy:mime()

    local part1 = alt:addpart()

    local part2 = alt:addpart()

    local part3 = mime:addpart()
    part3:subparts(alt, "multipart/alternative")

    a  = weak_ptr(alt)
    p1 = weak_ptr(part1)
    p2 = weak_ptr(part2)
    p3 = weak_ptr(part3)
  end

  gc_collect()

  assert_not_nil(a.value)
  assert_not_nil(p1.value)
  assert_not_nil(p2.value)
  assert_not_nil(p3.value)

  -- save reference to subnode
  local subnode = a.value

  mime = nil

  -- in this case call `free` to root node.
  -- there no way to get reference to this node from child
  -- so there no way to use it.
  gc_collect()

  -- libcurl still close all childs
  -- so all reference are invalid

  assert_not_nil(a.value)
  assert_not_nil(is_freed(a.value))
  assert_nil(p1.value)
  assert_nil(p2.value)
  assert_nil(p3.value)

  easy:close()
end

function test_preserve_mime_by_easy()

  local mime do
    mime = weak_ptr(easy:mime())
    easy:setopt_mimepost(mime.value)
  end

  gc_collect()

  assert_not_nil(mime.value)

  easy:unsetopt_mimepost()

  gc_collect()

  assert_nil(mime.value)
end

function test_mime_does_not_ref_to_easy()
  -- exists of mime object does not prevent easy from GC

  local mime = easy:mime()
  local peasy = weak_ptr(easy)
  easy = nil

  gc_collect()

  assert_nil(peasy.value)
end

function test_mimepost_does_not_ref_to_easy()
  -- exists of mime object does not prevent easy from GC

  -- create mime and set it as mimepost option
  local mime = easy:mime()
  easy:setopt_mimepost(mime)

  local peasy = weak_ptr(easy)
  easy = nil

  gc_collect()

  assert_nil(peasy.value)
end

end

local _ENV = TEST_CASE'mime basic' if not curl.OPT_MIMEPOST then
function test() skip("MIMI API supports since cURL 7.56.0") end
else

local easy, mime

local function dump_mime(mime)
  return dump_mime_(easy, mime, GET_URL)
end

function setup()
  easy = curl.easy()
  mime = easy:mime()
end

function teardown()
  if easy then easy:close() end
  if mime then mime:free() end
  easy, mime = nil
end

function test_data()
  mime:addpart():data('hello')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
end

function test_data_type()
  mime:addpart():data('hello', 'test/html')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
end

function test_data_type_name()
  mime:addpart():data('hello', 'test/html', 'test')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-%sname="test"', info)
end

function test_data_name()
  mime:addpart():data('hello', nil, 'test')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Disposition:.-%sname="test"', info)
end

function test_data_filename()
  mime:addpart():data('hello', nil, nil, 'test.html')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Disposition:.-%sfilename="test%.html"', info)
end

function test_data_should_not_unset_on_nil()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  })
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)

  part:data('world', nil, 'test2')
  info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nworld', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test2"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)

  part:data('!!!!!', 'text/xml', nil)
  info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\n!!!!!', info)
  assert_match('Content%-Type:%s+text/xml', info)
  assert_match('Content%-Disposition:.-name="test2"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)

  part:data('!!!!!!!', 'text/xml', nil, nil)
  info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\n!!!!!', info)
  assert_match('Content%-Type:%s+text/xml', info)
  assert_match('Content%-Disposition:.-name="test2"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_data_headers()
  mime:addpart():data('hello', {
    'X-Custom-Header: hello'
  })
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_name()
  mime:addpart():data('hello', 'test/html', 'test'):name(false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
end

function test_unset_name_by_null()
  mime:addpart():data('hello', 'test/html', 'test'):name(null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
end

function test_unset_type()
  mime:addpart():data('hello', 'test/html'):type(false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+test/html', info)
end

function test_unset_type_by_null()
  mime:addpart():data('hello', 'test/html'):type(null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+test/html', info)
end

function test_unset_headers()
  mime:addpart():data('hello', 'test/html',{
    'X-Custom-Header: hello'
  }):headers(false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_headers_by_null()
  mime:addpart():data('hello', 'test/html',{
    'X-Custom-Header: hello'
  }):headers(null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_headers_by_empty_array()
  mime:addpart():data('hello', 'test/html',{
    'X-Custom-Header: hello'
  }):headers({})

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data()
  mime:addpart():data('hello', 'test/html', 'test'):data(false)

  local info = assert_string(dump_mime(mime))
  assert_not_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
end

function test_unset_data_by_null()
  mime:addpart():data('hello', 'test/html', 'test'):data(null)

  local info = assert_string(dump_mime(mime))
  assert_not_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
end

function test_unset_data_type_1()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_type_1_by_null()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_type_2()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', false, nil, nil)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_name_1()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', nil, false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_name_1_by_null()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', nil, null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_name_2()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', nil, false, nil)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_header()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', nil, nil, nil, false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_not_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_header_by_null()
  local part = mime:addpart():data('hello', 'test/html', 'test', {
    'X-Custom-Header: hello'
  }):data('hello', nil, nil, nil, null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
  assert_not_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_filename_1()
  local part = mime:addpart():data('hello', 'test/html', 'test', 'test.html', {
    'X-Custom-Header: hello'
  }):data('hello', nil, nil, false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-%sname="test"', info)
  assert_not_match('Content%-Disposition:.-%sfilename="test%.html"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_unset_data_filename_1_by_null()
  local part = mime:addpart():data('hello', 'test/html', 'test', 'test.html', {
    'X-Custom-Header: hello'
  }):data('hello', nil, nil, null)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-%sname="test"', info)
  assert_not_match('Content%-Disposition:.-%sfilename="test%.html"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
end

function test_fail_pass_nil_as_first_arg()
  local part = mime:addpart()
  assert_error(function() part:data()    end)
  assert_error(function() part:data(nil) end)

  assert_error(function() part:name()    end)
  assert_error(function() part:name(nil) end)

  assert_error(function() part:type()    end)
  assert_error(function() part:type(nil) end)

  assert_error(function() part:headers()    end)
  assert_error(function() part:headers(nil) end)
end

end

local _ENV = TEST_CASE'mime addpart' if not curl.OPT_MIMEPOST then
function test() skip("MIMI API supports since cURL 7.56.0") end
else

local easy, mime

local function dump_mime(mime)
  return dump_mime_(easy, mime, GET_URL)
end

function setup()
  easy = curl.easy()
  mime = easy:mime()
end

function teardown()
  if easy then easy:close() end
  if mime then mime:free() end
  easy, mime = nil
end

function test_empty_table()
  assert(mime:addpart{})
end

function test_pass_args()
  assert(mime:addpart{
    data = 'hello';
    encoder = 'base64';
    name = 'test';
    filename = 'test.html';
    type = 'test/html';
    headers = {
      'X-Custom-Header: hello';
    }
  })

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\naGVsbG8=', info)
  assert_match('Content%-Type:%s+test/html', info)
  assert_match('Content%-Disposition:.-%sname="test"', info)
  assert_not_match('Content%-Disposition:.-%sname="test%.html"', info)
  assert_match('X%-Custom%-Header:%s*hello', info)
  assert_match('Content%-Transfer%-Encoding:%s*base64', info)
end

end

RUN()
