local cURL = require("cURL")

local urls = {
  "http://httpbin.org/get?key=1",
  "http://httpbin.org/get?key=2",
  "http://httpbin.org/get?key=3",
  "http://httpbin.org/get?key=4",
}

local function next_easy()
  local url = table.remove(urls, 1)
  if url then return cURL.easy{url = url} end
end

m = cURL.multi():add_handle(next_easy())
for data, type, easy in m:iperform() do

  if type == "done" or type == "error" then 
    print("Done", easy:getinfo_effective_url(), ":", data)
    easy:close()
    easy = next_easy()
    if easy then m:add_handle(easy) end
  end

  if type == "data" then print(data) end

end

