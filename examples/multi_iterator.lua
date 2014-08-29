--
local function multi_iterator(...)
  local curl = require "lcurl.safe"

  local buffers = {_ = {}} do

  function buffers:append(e, ...)
    local b = self._[e] or {}
    self._[e] = b
    b[#b + 1] = {...}
  end

  function buffers:next()
    for e, t in pairs(self._) do
      local m = table.remove(t, 1)
      if m then return e, m end
    end
  end

  end

  local function multi_init(...)
    local remain = 0
    local m      = curl.multi()
    for _, e in ipairs{...} do
      e:setopt_writefunction(function(str) buffers:append(e, "data", str) end)
      e:setopt_headerfunction(function(str) buffers:append(e, "header", str) end)
      m:add_handle(e)
      remain = remain + 1
    end
    return m, remain
  end

  local m, remain = multi_init(...)
  m:perform()
  return function()
    while true do
      local e, t = buffers:next()
      if t then return e, unpack(t) end
      if remain == 0 then break end

      m:wait()

      local n, err = m:perform()
      if not n then m:close() error(err) end

      if n <= remain then
        while true do
          local e, ok, err = m:info_read()
          if not e then m:close() error(err) end
          if e == 0 then break end
          if ok then buffers:append(e, "done", ok)
          else buffers:append(e, "error", err) end
        end
        remain = n
      end

    end
    m:close()
  end
end

--
local curl = require "lcurl"

c1 = curl.easy()
  :setopt_url("http://www.lua.org/")

c2 = curl.easy()
  :setopt_url("http://luajit.org/")

for easy, type, data in multi_iterator(c1, c2) do
      if type == 'header' then print(easy, type, (data:gsub("%s*$", "")))
  elseif type == 'data'   then print(easy, type, #data)
  elseif type == 'error'  then print(easy, type, data)
  elseif type == 'done'   then print(easy, type, data)
  end
end
