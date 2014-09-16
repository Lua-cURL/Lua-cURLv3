--
--  Author: Alexey Melnichuk <mimir@newmail.ru>
--
--  Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
--
--  Licensed according to the included 'LICENSE' document
--
--  This file is part of Lua-cURL library.
--

local curl = require "lcurl"
local impl = require "cURL.impl.cURL"

return impl(curl)
