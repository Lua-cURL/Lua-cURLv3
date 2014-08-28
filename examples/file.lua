local cURL = require("lcurl")

-- open output file
f = assert(io.open("example_homepage.html", "w"))

cURL.easy()
  -- setup url
  :setopt_url("http://www.example.com/")
  -- setup file object as writer
  :setopt_writefunction(f)
  -- perform, invokes callbacks
  :perform()
-- close easy
:close()

-- close output file
f:close()
print("Done")