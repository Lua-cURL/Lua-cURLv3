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

local dump_mime_url = 'http://127.0.0.1:7090'

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

end

local _ENV = TEST_CASE'mime basic' if not curl.OPT_MIMEPOST then
function test() skip("MIMI API supports since cURL 7.56.0") end
else

local easy, mime

local function dump_mime(mime)
  return dump_mime_(easy, mime, dump_mime_url)
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
  mime:addpart():data('hello', 'text/html')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+text/html', info)
end

function test_data_type_name()
  mime:addpart():data('hello', 'text/html', 'test')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+text/html', info)
  assert_match('Content%-Disposition:.-name="test"', info)
end

function test_data_name()
  mime:addpart():data('hello', nil, 'test')
  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Disposition:.-name="test"', info)
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
  mime:addpart():data('hello', 'text/html', 'test'):name(false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_match('Content%-Type:%s+text/html', info)
  assert_not_match('Content%-Disposition:.-name="test"', info)
end

function test_unset_type()
  mime:addpart():data('hello', 'text/html'):type(false)

  local info = assert_string(dump_mime(mime))
  assert_match('\r\n\r\nhello', info)
  assert_not_match('Content%-Type:%s+text/html', info)
end

end

RUN()
