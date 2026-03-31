local lmsgpack = require "lmsgpack"
local msgpack = require "msgpack"
require "util"

local obj = {
    i = 100,
    si = -10,
    -- d = 10.0,
    str = "string",
    arr = { 10, 20, { [100] = 200, arr = { 100, 200 } } },
    [100] = { id = 100, val = 200 }
}
local test = function()
    local bin = lmsgpack.encode(obj)
    print(#bin, #msgpack.encode(obj))
    print(dump(lmsgpack.decode(bin)))
end

local press = function()
    local bin
    local t = os.time()
    for i = 1, 1000000 do
        bin = lmsgpack.encode(obj)
    end
    print(os.time() - t)

    local t = os.time()
    for i = 1, 1000000 do
        lmsgpack.decode(bin)
    end
    print(os.time() - t)
end
press()
