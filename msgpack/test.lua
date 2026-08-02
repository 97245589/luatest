local lmsgpack = require "lmsgpack"
local msgpack = require "msgpack"
require "util"

local obj = {
    i = 100,
    si = -10,
    d = 10.10,
    sd = -10.10,
    str = "string",
    arr = { 10, 20, { [100] = 200, arr = { 100, 200 } } },
    map = { [100] = { id = 100, val = 200 } }
}
local test = function()
    local enobj = lmsgpack.create(1024 * 1024)
    local bin = enobj:encode(obj)
    print(#bin, #msgpack.encode(obj))
    print(dump(lmsgpack.decode(bin)))

    local press = function()
        local t = os.time()
        for i = 1, 1000000 do
            bin = enobj:encode(obj)
        end
        print(os.time() - t, #bin)

        local t = os.time()
        for i = 1, 1000000 do
            lmsgpack.decode(bin)
        end
        print(os.time() - t)
    end
    press()
end
test()
