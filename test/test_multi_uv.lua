#!/usr/bin/env lua

local curl = require('lcurl')
local uv = require('luv')

local function uvwget(...)
    local wgets = {}
    local m = curl.multi()

    local function check_multi_info()
        while true do
            local easy, done, err = m:info_read(true)
            if type(easy) == "number" then
                break
            end

            if done then
                local w = wgets[easy]
                if w.data then
                    w.data = table.concat(w.data)
                end
                if w.oncomplete then
                    w.oncomplete(easy, w.data)
                end
                wgets[easy] = nil
                easy:close()
            end
        end
    end

    local timer = uv.new_timer()

    local function start_timer(ms)
        if ms <= 0 then
            ms = 1
        end
        timer:start(ms, 0, function ()
            m:socket_action(curl.SOCKET_TIMEOUT, 0)
            check_multi_info()
        end)
    end

    local fdpoll = {}

    local function handle_socket(easy, fd, action)
        local poll = fdpoll[fd]
        if not poll then
            poll = uv.new_socket_poll(fd)
            fdpoll[fd] = poll
        end

        local function curl_perform(status, events)
            timer:stop()
            local evt = 0
            if events == "r" then
                evt = curl.CSELECT_IN
            elseif events == "w" then
                evt = curl.CSELECT_OUT
            elseif events == "rw" then
                evt = curl.CSELECT_IN + curl.CSELECT_OUT
            end
            m:socket_action(fd, evt)
            check_multi_info()
        end

        if action == curl.POLL_IN then
            poll:start("r", curl_perform)
        elseif action == curl.POLL_OUT then
            poll:start("w", curl_perform)
        elseif action == curl.POLL_REMOVE then
            poll:stop()
            fdpoll[fd] = nil
        else
        end
    end

    m:setopt(curl.OPT_MULTI_SOCKETFUNCTION, handle_socket)
    m:setopt_timerfunction(start_timer)

    -- walk through
    for _, wget in ipairs({...}) do
        local easy = curl.easy()
                        :setopt_url(wget.url)
        if wget.post then
            if type(wget.post) == "string" then
                easy:setopt_postfields(wget.post)
            else
                easy:setopt_httppost(wget.post)
            end
        end

        wget.data = {}
        easy:setopt_writefunction(function(ud, s)
            table.insert(ud, s)
            return true
        end, wget.data)

        m:add_handle(easy)
        wgets[easy] = wget
    end
end

---------------------------------------------------------------------

local start_time = os.time()

local function callback_done(easy, data)
    local url = easy:getinfo(curl.INFO_EFFECTIVE_URL)
    print(url, " -> DONE")
    if data then
        print(string.len(data), data)
    end
end

uvwget({
    url = [[http://pkg-shadow.alioth.debian.org/releases/shadow-4.2.1.tar.xz]],
    filename = true,
    oncomplete = callback_done
}, {
    url = [[http://www.infodrom.org/projects/sysklogd/download/sysklogd-1.5.1.tar.gz]],
    filename = true,
    oncomplete = callback_done
}, {
    url = [[http://www.zlib.net/zlib-1.2.8.tar.xz]],
    filename = true,
    oncomplete = callback_done
}, {
    url = [[http://ipinfo.io]],
    oncomplete = function(easy, data)
        print(data)
    end
})


uv.run()

local elapse = os.time() - start_time
print("elapse = " .. elapse .. " s")
