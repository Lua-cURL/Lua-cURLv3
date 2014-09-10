-- Cookie data will be shared across the easy handles to do an authorized download
local cURL = require("lcurl")

-- create share handle (share COOKIE and DNS Cache)
s = cURL.share()
  :setopt_share(cURL.LOCK_DATA_COOKIE      )
  :setopt_share(cURL.LOCK_DATA_DNS         )

-- create first easy handle to do the login
c = cURL.easy()
  :setopt_share(s)
  :setopt_url("http://targethost/login.php?username=foo&password=bar")

-- create second easy handle to do the download
c2 = cURL.easy()
  :setopt_share(s)
  :setopt_url("http://targethost/download.php?id=test")

-- login
c:perform()

-- download
c2:perform()
