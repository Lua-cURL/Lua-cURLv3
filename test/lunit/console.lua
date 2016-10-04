
--[[--------------------------------------------------------------------------

    This file is part of lunit 0.6.

    For Details about lunit look at: http://www.mroth.net/lunit/

    Author: Michael Roth <mroth@nessie.de>

    Copyright (c) 2006-2008 Michael Roth <mroth@nessie.de>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

--]]--------------------------------------------------------------------------



--[[

      begin()
        run(testcasename, testname)
          err(fullname, message, traceback)
          fail(fullname, where, message, usermessage)
          pass(testcasename, testname)
      done()

      Fullname:
        testcase.testname
        testcase.testname:setupname
        testcase.testname:teardownname

--]]


local lunit  = require "lunit"
local string = require "string"
local io     = require "io"
local table  = require "table"

local _M = {}

local function rfill(str, wdt, ch)
  if wdt > #str then str = str .. (ch or ' '):rep(wdt - #str) end
  return str
end

local function printformat(format, ...)
  io.write( string.format(format, ...) )
end

local columns_printed = 0

local function writestatus(char)
  if columns_printed == 0 then
    io.write("    ")
  end
  if columns_printed == 60 then
    io.write("\n    ")
    columns_printed = 0
  end
  io.write(char)
  io.flush()
  columns_printed = columns_printed + 1
end

local msgs = {}

function _M.begin()
  local total_tc = 0
  local total_tests = 0

  msgs = {} -- e

  for tcname in lunit.testcases() do
    total_tc = total_tc + 1
    for testname, test in lunit.tests(tcname) do
      total_tests = total_tests + 1
    end
  end

  printformat("Loaded testsuite with %d tests in %d testcases.\n\n", total_tests, total_tc)
end

function _M.run(testcasename, testname)
  io.write(rfill(testcasename .. '.' .. testname, 70)) io.flush()
end

function _M.err(fullname, message, traceback)
  io.write(" - error!\n")
  io.write("Error! ("..fullname.."):\n"..message.."\n\t"..table.concat(traceback, "\n\t"), "\n")
end

function _M.fail(fullname, where, message, usermessage)
  io.write(" - fail!\n")
  io.write(string.format("Failure (%s): %s\n%s: %s", fullname, usermessage or "", where, message), "\n")
end

function _M.skip(fullname, where, message, usermessage)
  io.write(" - skip!\n")
  io.write(string.format("Skip (%s): %s\n%s: %s", fullname, usermessage or "", where, message), "\n")
end

function _M.pass(testcasename, testname)
  io.write(" - pass!\n")
end

function _M.done()
  printformat("\n\n%d Assertions checked.\n", lunit.stats.assertions )
  print()

  printformat("Testsuite finished (%d passed, %d failed, %d errors, %d skipped).\n",
      lunit.stats.passed, lunit.stats.failed, lunit.stats.errors, lunit.stats.skipped )
end

return _M
