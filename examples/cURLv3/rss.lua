-- use LuaExpat and Lua-CuRL together for On-The-Fly XML parsing
local lxp  = require "lxp"
local cURL = require "cURL"

-- create XML parser
items, tags  = {}, {}
p = lxp.new{
   StartElement  = function (parser, tagname)
      tags[#tags + 1] = tagname
      if (tagname == "item") then
         items[#items + 1] = {}
      end
   end;

   CharacterData = function (parser, str)
      if (tags[#tags -1] == "item") then
         --we are parsing a item, get rid of trailing whitespace
         items[#items][tags[#tags]] = string.gsub(str, "%s*$", "")
      end
   end;

   EndElement    = function (parser, tagname)
      --assuming well formed xml
      tags[#tags] = nil
   end;
}

-- create and setup easy handle
c = cURL.easy{url = "http://www.lua.org/news.rss"}

-- setup writer function with context
c:setopt_writefunction(p.parse, p)

-- perform request and close easy handle
-- perform raise error if parser fail
c:perform():close()

--finish document
assert(p:parse())
p:close()

for i, item in ipairs(items) do 
   for k, v in pairs(item) do 
      print(k,v)
   end
   print()
end