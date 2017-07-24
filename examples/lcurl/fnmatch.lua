--
-- Example of how to download multiple files in single perform
--

local curl = require "lcurl"

local function printf(...)
  io.stderr:write(string.format(...))
end

local function pat2pat(s)
  return "^" .. string.gsub(s, ".", {
    ["*"] = '[^%.]*';
    ["."] = '%.';
  }) .. "$"
end

local c = curl.easy{
  url = "ftp://moteus:123456@127.0.0.1/test.*";
  wildcardmatch = true;
}

local data, n = 0, 0
c:setopt_writefunction(function(chunk) data = #chunk + data end)

-- called before each new file
c:setopt_chunk_bgn_function(function(info, remains)
  data, n = 0, n + 1
  printf('\n======================================================\n')
  printf('new file `%s` #%d, remains - %d\n', info.filename, n, remains or -1)
end)

-- called after file download complite
c:setopt_chunk_end_function(function()
  printf('total size %d[B]\n', data)
  printf('------------------------------------------------------\n')
end)

-- custom pattern matching function
c:setopt_fnmatch_function(function(pattern, name)
  local p = pat2pat(pattern)
  local r = not not string.match(name, p)
  printf("%s %s %s\n", r and '+' or '-', pattern, name)
  return r
end)

c:perform()
