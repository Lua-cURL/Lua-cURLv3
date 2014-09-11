local cURL = require "cURL"

-- open output file
f = io.open("example_homepage", "w")

cURL.easy{
  url = "http://www.example.com/",
  writefunction = f
}
:perform()
:close()

-- close output file
f:close()

print("Done")