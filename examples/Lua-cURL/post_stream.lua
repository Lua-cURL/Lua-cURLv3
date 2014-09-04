local cURL = require("lcurl.cURL")

c = cURL.easy_init()

c:setopt_url("http://posttestserver.com/post.php")

c:setopt_post(true)

c:post{-- post file from private read function
  -- Lua-cURL compatiable
  -- allows only one stream
  name = {
    file="stream.txt",
    stream_length="5",
    type="text/plain",
  }
}

count = 0
c:perform{readfunction = function(n)
  if count < 5  then
    count = 5
    return "stream"
  end
  return nil
end}

c:post{-- post file from private read function
  -- define stream callback
  name = {
    file          = "stream.txt",
    type          = "text/plain",
    stream_length = 5,
    stream        = function()
      if count < 5  then
        count = 5
        return "STREAM"
      end
      return nil
    end,
  }
}

count = 0
c:perform{}

print("Done")
