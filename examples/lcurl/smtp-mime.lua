local curl = require "lcurl"

local SMTP = {
  url      = "smtp://mail.example.com";
}

local FROM      = "<sender@example.org>"
local TO        = "<addressee@example.net>"
local CC        = "<info@example.org>"
local FILE      = "smtp-mime.lua" -- if you send this file do not forget it may have mail password
local CT_FILE   = "application/lua"

local DUMP_MIME = false

local headers = {
  "Date: Tue, 22 Aug 2017 14:08:43 +0100",
  "To: "   .. TO,
  "From: " .. FROM .. " (Example User)",
  "Cc: "   .. CC   .. " (Another example User)",
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>",
  "Subject: example sending a MIME-formatted message",
}

local inline_text = ""
  .. "This is the inline text message of the e-mail.\r\n"
  .. "\r\n"
  .. "  It could be a lot of lines that would be displayed in an e-mail\r\n"
  .. "viewer that is not able to handle HTML.\r\n"

local inline_html = ""
  .. "<html><body>\r\n"
  .. "<p>This is the inline <b>HTML</b> message of the e-mail.</p>"
  .. "<br />\r\n"
  .. "<p>It could be a lot of HTML data that would be displayed by "
  .. "e-mail viewers able to handle HTML.</p>"
  .. "</body></html>\r\n"

local function dump_mime(type, data)
  if type == curl.INFO_DATA_OUT then io.write(data) end
end

local easy = curl.easy()

local mime = easy:mime() do
  local alt = easy:mime()
  alt
    :addpart()
      :data(inline_html, "text/html")
  alt
    :addpart()
      :data(inline_text)
  mime:addpart()
    :subparts(alt, "multipart/alternative", {
      "Content-Disposition: inline"
    })
  mime
    :addpart()
      :filedata(FILE, CT_FILE)
end

easy:setopt{
  url            = SMTP.url,
  mail_from      = FROM,
  mail_rcpt      = {TO, CC},
  httpheader     = headers;
  mimepost       = mime;
  ssl_verifyhost = false;
  ssl_verifypeer = false;
  username       = SMTP.user;
  password       = SMTP.password;
  upload         = true;
}

if DUMP_MIME then
  easy:setopt{
    verbose        = true;
    debugfunction  = dump_mime;
  }
end

easy:perform()

easy:close()

mime:free()
