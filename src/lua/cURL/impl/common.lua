local common = {}
function common.hash_id(str)
	local id = string.match(str, "%((.-)%)") or string.match(str, ': (%x+)$')
	return id
end

function common.clone(t, o)
	o = o or {}
	for k, v in pairs(t) do o[k] = v end
	return o
end

function common.wrap_function(k)
	return function(self, ...)
		local ok, err = self._handle[k](self._handle, ...)
		if ok == self._handle then return self end
		return ok, err
	end
end

function common.new_buffers()
	local buffers = { resp = {}, _ = {} }

	function buffers:append(e, ...)
		local resp = assert(e:getinfo_response_code())
		if not self._[e] then self._[e] = {} end

		local b = self._[e]

		if self.resp[e] ~= resp then
			b[#b + 1] = { "response", resp }
			self.resp[e] = resp
		end

		b[#b + 1] = { ... }
	end

	function buffers:next()
		for e, t in pairs(self._) do
			local m = table.remove(t, 1)
			if m then return e, m end
		end
	end

	return buffers
end

function common.make_iterator(self, perform)
	local curl = require "lcurl.safe"

	local buffers = common.new_buffers()

	-- reset callbacks to all easy handles
	local function reset_easy(self)
		if not self._easy_mark then -- that means we have some new easy handles
			for h, e in pairs(self._easy) do if h ~= 'n' then
					e:setopt_writefunction(function(str) buffers:append(e, "data", str) end)
					e:setopt_headerfunction(function(str) buffers:append(e, "header", str) end)
				end
			end
			self._easy_mark = true
		end
		return self._easy.n
	end

	if 0 == reset_easy(self) then return end

	assert(perform(self))

	return function()
		-- we can add new handle during iteration
		local remain = reset_easy(self)

		-- wait next event
		while true do
			local e, t = buffers:next()
			if t then return t[2], t[1], e end
			if remain == 0 then break end

			self:wait()

			local n = assert(perform(self))

			if n <= remain then
				while true do
					local e, ok, err = assert(self:info_read())
					if e == 0 then break end
					if ok then
						ok = e:getinfo_response_code() or ok
						buffers:append(e, "done", ok)
					else buffers:append(e, "error", err) end
					self:remove_handle(e)
					e:unsetopt_headerfunction()
					e:unsetopt_writefunction()
				end
			end

			remain = n
		end
	end
end

function common.form_add_element(form, name, value)
	local vt = type(value)
	if vt == "string" then return form:add_content(name, value) end

	assert(type(name) == "string")
	assert(vt == "table")
	assert((value.name == nil) or (type(value.name) == 'string'))
	assert((value.type == nil) or (type(value.type) == 'string'))
	assert((value.headers == nil) or (type(value.headers) == 'table'))

	if value.stream then
		local vst = type(value.stream)

		if vst == 'function' then
			assert(type(value.length) == 'number')
			local length = value.length
			return form:add_stream(name, value.name, value.type, value.headers, length, value.stream)
		end

		if (vst == 'table') or (vst == 'userdata') then
			local length = value.length or assert(value.stream:length())
			assert(type(length) == 'number')
			return form:add_stream(name, value.name, value.type, value.headers, length, value.stream)
		end

		error("Unsupported stream type: " .. vst)
	end

	if value.file then
		assert(type(value.file) == 'string')
		return form:add_file(name, value.file, value.type, value.filename, value.headers)
	end

	if value.data then
		assert(type(value.data) == 'string')
		assert(type(value.name) == 'string')
		return form:add_buffer(name, value.name, value.data, value.type, value.headers)
	end

	local content = value[1] or value.content
	if content then
		assert(type(content) == 'string')
		if value.type then
			return form:add_content(name, content, value.type, value.headers)
		end
		return form:add_content(name, content, value.headers)
	end

	return form
end

function common.form_add(form, data)
	for k, v in pairs(data) do
		local ok, err = common.form_add_element(form, k, v)
		if not ok then return nil, err end
	end

	return form
end

function common.class(ctor, type_name)
	local C = {}
	C.__type = type_name or "LcURL Unknown"
	C.__index = function(self, k)
		local fn = C[k]

		if not fn and self._handle[k] then
			fn = common.wrap_function(k)
			C[k] = fn
		end
		return fn
	end

	function C:new(...)
		local h, err = ctor()
		if not h then return nil, err end

		local o = setmetatable({
			_handle = h
		}, self)

		if self.__init then return self.__init(o, ...) end

		return o
	end

	function C:handle()
		return self._handle
	end

	return C
end

return common
