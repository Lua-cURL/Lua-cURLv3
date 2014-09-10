-- simple "On the fly" fileupload
local cURL = require("lcurl")

count=0
cURL.easy()
  :setopt_url("ftp://ftptest:secret0815@targethost/file.dat")
  :setopt_upload(true)
  :setopt_readfunction(function()
    count = count + 1
    if count < 10 then
      return "Line " .. count .. "\n"
    end
  end)
  :perform()
:close()

print("Fileupload done")