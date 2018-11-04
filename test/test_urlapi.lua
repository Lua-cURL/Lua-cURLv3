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

local _, luacov   = pcall(require, "luacov")
local TEST_CASE   = assert(lunit.TEST_CASE)
local skip        = lunit.skip or function() end
local curl        = require "cURL"
local scurl       = require "cURL.safe"
local utils       = require "utils"

local tostring    = tostring

local ENABLE = true

local _ENV = TEST_CASE'urlapi' if ENABLE then

if not curl.E_URL_OK then skip('URL API avaliable since libcurl 7.62.0') else

local it = setmetatable(_ENV or _M, {__call = function(self, describe, fn)
  self["test " .. describe] = fn
end})

local url

local function U(u)
  url = assert_userdata(curl.url())
  assert_equal(url, url:set_url(u))
  return url
end

function teardown()
  if url then url:cleanup() end
  url = nil
end

it('should export falgs', function()
  assert_number(curl.U_DEFAULT_PORT        )
  assert_number(curl.U_NO_DEFAULT_PORT     )
  assert_number(curl.U_DEFAULT_SCHEME      )
  assert_number(curl.U_NON_SUPPORT_SCHEME  )
  assert_number(curl.U_PATH_AS_IS          )
  assert_number(curl.U_DISALLOW_USER       )
  assert_number(curl.U_URLDECODE           )
  assert_number(curl.U_URLENCODE           )
  assert_number(curl.U_APPENDQUERY         )
  assert_number(curl.U_GUESS_SCHEME        )
end)

it('should export parts', function()
  assert_number(curl.UPART_URL      )
  assert_number(curl.UPART_SCHEME   )
  assert_number(curl.UPART_USER     )
  assert_number(curl.UPART_PASSWORD )
  assert_number(curl.UPART_OPTIONS  )
  assert_number(curl.UPART_HOST     )
  assert_number(curl.UPART_PORT     )
  assert_number(curl.UPART_PATH     )
  assert_number(curl.UPART_QUERY    )
  assert_number(curl.UPART_FRAGMENT )
end)

it('should export methods', function()
  url = curl.url()
  assert_function(url.dup          )
  assert_function(url.cleanup      )

  assert_function(url.set_url      )
  assert_function(url.set_scheme   )
  assert_function(url.set_user     )
  assert_function(url.set_password )
  assert_function(url.set_options  )
  assert_function(url.set_host     )
  assert_function(url.set_port     )
  assert_function(url.set_path     )
  assert_function(url.set_query    )
  assert_function(url.set_fragment )

  assert_function(url.get_url      )
  assert_function(url.get_scheme   )
  assert_function(url.get_user     )
  assert_function(url.get_password )
  assert_function(url.get_options  )
  assert_function(url.get_host     )
  assert_function(url.get_port     )
  assert_function(url.get_path     )
  assert_function(url.get_query    )
  assert_function(url.get_fragment )
end)

it('create and cleanup', function()
  url = assert_userdata(curl.url())
  assert_nil(url:cleanup())
end)

it('constructor with parameters', function()
  url = assert_userdata(curl.url('http://example.com/'))
  assert_equal('http://example.com/', url:get_url())

  url = assert_userdata(curl.url('example.com', curl.U_GUESS_SCHEME))
  assert_equal('http://example.com/', url:get_url())
end)

it('dup url', function()
  url = assert_userdata(curl.url('http://example.com/'))
  local u2 = url:dup()
  assert_not_equal(url, u2)
  assert_equal('http://example.com/', u2:get_url())
  assert_equal('http://example.com/', url:get_url())
  url:cleanup()
  url = u2
  assert_equal('http://example.com/', url:get_url())
end)

it('should convert to string', function()
  assert_equal('http://example.com/', tostring(U"http://example.com/"))
end)

it('should decode url', function()
  url = U"http://example.com"
  assert_equal('http',        url:get_scheme())
  assert_equal('example.com', url:get_host())
  assert_equal('/',           url:get_path())
end)

it('should cast scheme to lower case', function()
  url = U"HTTP://Example.com"
  assert_equal('http',                url:get_scheme())
  assert_equal('Example.com',         url:get_host())
  assert_equal('/',                   url:get_path())
  assert_equal("http://Example.com/", url:get_url())
end)

it('should append query', function()
  url = U"http://example.com"
  assert_equal(url, url:set_query("a=hello world", curl.U_APPENDQUERY + curl.U_URLENCODE))
  assert_equal(url, url:set_query("b=A&B", curl.U_APPENDQUERY + curl.U_URLENCODE))
  assert_equal("http://example.com/?a=hello+world&b=A%26B", url:get_url())
end)

it('should append only one parameter in query per call', function()
  url = U"http://example.com"
  assert_equal(url, url:set_query("a=hello world&b=A&B", curl.U_APPENDQUERY + curl.U_URLENCODE))
  assert_equal("http://example.com/?a=hello+world%26b=A%26B", url:get_url())
end)

it('should set encoded query', function()
  url = U("http://example.com/?a=hello world&b=d")
  assert_equal('a=hello world&b=d', url:get_query())
end)

it('should returns NULL as empty value', function()
  url = curl.url()
  assert_equal(curl.null, url:get_query())
  assert_equal(curl.null, url:get_host())
  assert_equal(curl.null, url:get_port())
  assert_equal(curl.null, url:get_password())
  assert_equal(curl.null, url:get_scheme())
  assert_equal(curl.null, url:get_options())
  assert_equal(curl.null, url:get_fragment())

  assert_equal('/', url:get_path())
end)

it('should returns nil and error for invalid url in safe mode', function()
  url = scurl.url()
  local _, err = assert_nil(url:get_url())
  assert_equal('CURL-URL', err:cat())
end)

it('should raise error for invalid url', function()
  url = curl.url()
  assert_error_match('CURL%-URL', function() url:get_url() end)
end)

-- it('should set encoded query', function()
--   url = U"http://example.com"
--   assert_equal(url, url:set_query("a=hello world", curl.U_URLENCODE))
--   assert_equal("http://example.com/?a=hello+world", url:get_url())
-- end)

end end

RUN()
