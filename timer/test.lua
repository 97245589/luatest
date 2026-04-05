require "util"
local ltimer = require "ltimer"

local core = ltimer.create()

local test = function()
    for i = 1, 5 do
        core:add(1, i, i)
        core:add(2, i, i)
    end
    core:add(1, 10, 1)
    print(core:dump())
    core:del(2, 2)
    core:del(1, 3)
    print(core:dump())
    core:delid(2)
    print(core:dump())
    print(dump(core:expire(5)))
    print(core:dump())
end

local test = function()
    local t = os.time()
    for i = 1, 1000000 do
        core:add(1, 1000000 - i, i)
    end
    core:delid(1)
    print(core:dump())
    print(os.time() - t)
end
test()