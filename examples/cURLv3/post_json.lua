local cURL = require "cURL"

c = cURL.easy{
  url        = "http://posttestserver.com/post.php",
  post       = true,
  httpheader = {
    "Content-Type: application/json";
  };
  postfields = '{"hello": "world"}';
}

c:perform()