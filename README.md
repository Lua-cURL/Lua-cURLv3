# Lua binding to [libcurl](http://curl.haxx.se/libcurl)
[![Build Status](https://travis-ci.org/Lua-cURL/lua-lcurl.png?branch=master)](https://travis-ci.org/Lua-cURL/lua-lcurl)
[![Coverage Status](https://coveralls.io/repos/Lua-cURL/lua-lcurl/badge.png?branch=master)](https://coveralls.io/r/Lua-cURL/lua-lcurl?branch=master)
[![Licence](http://img.shields.io/badge/Licence-MIT-brightgreen.svg)](LICENSE)

## Documentation
[API](http://moteus.github.com/lcurl)<br/>
Also library provide `lcurl.cURL` compatibility module for [Lua-cURL](http://lua-curl.github.io/Lua-cURL) binding.

## Why one more curl binding

Existing [Lua-cURL](http://lua-curl.github.io/Lua-cURL) binding has several problems:

* it can not return error codes but just raise Lua errors
* it raise Lua error from callback that may result resource leak in libcurl
* it does not provide building multipart/formdata explicitly
* it has memory leak when send multipart/formdata
* it does not save string for curl options that may result crush in libcurl
* there no way to get result for operations in multi interface (e.g. if one of easy operation fail you can not get result code/error message)
* you can not use multi interface for upload operation
* you can not use your own callback function to perform operation with multi interface
* you can not pass your context to callback functions

## Installation

```
luarocks install lcurl --server=https://rocks.moonscript.org/dev
```

## Usage

```Lua
-- HTTP Get
curl.easy()
  :setopt_url('http://httpbin.org/get')
  :setopt_httpheader{
    "X-Test-Header1: Header-Data1",
    "X-Test-Header2: Header-Data2",
  }
  :setopt_writefunction(io.stderr) -- use io.stderr:write()
  :perform()
:close()
```

```Lua
-- HTTP Post
curl.easy()
  :setopt_url('http://posttestserver.com/post.php')
  :setopt_writefunction(io.write)
  :setopt_httppost(curl.form() -- lcurl guarantee that form will be alive
    :add_content("test_content", "some data", {
      "MyHeader: SomeValue"
    })
    :add_buffer("test_file", "filename", "text data", "text/plain", {
      "Description: my file description"
    })
    :add_file("test_file2", "BuildLog.htm", "application/octet-stream", {
      "Description: my file description"
    })
  )
  :perform()
:close()
```

```Lua
-- FTP Upload
local function get_bin_by(str,n)
  local pos = 1 - n
  return function()
    pos = pos + n
    return (str:sub(pos,pos+n-1))
  end
end

curl.easy()
  :setopt_url("ftp://moteus:123456@127.0.0.1/test.dat")
  :setopt_upload(true)
  :setopt_readfunction(
    get_bin_by(("0123456789"):rep(4), 9)
  )
  :perform()
:close()
```

```Lua
-- Multi FTP Upload

-- We get error E_LOGIN_DENIED for this operation
e1 = curl.easy()
  :setopt_url("ftp://moteus:999999@127.0.0.1/test1.dat")
  :setopt_upload(true)
  :setopt_readfunction(
    function(t) return table.remove(t) end, {"1111", "2222"}
  )

e2 = curl.easy()
  :setopt_url("ftp://moteus:123456@127.0.0.1/test2.dat")
  :setopt_upload(true)
  :setopt_readfunction(get_bin_by(("e"):rep(1000), 5))

m = curl.multi()
m:add_handle(e1)
m:add_handle(e2)

while m:perform() > 0 do m:wait() end

while true do
  h, ok, err = m:info_read()
  if h == 0 then break end

  if h == e1 then 
    assert(ok == nil)
    assert(err:name() == "LOGIN_DENIED")
    assert(err:no() == curl.E_LOGIN_DENIED)
  end

  if h == e2 then 
    assert(ok == true)
  end
end
```

