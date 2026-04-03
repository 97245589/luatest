require "util"
local lworld = require "lworld"
local core = lworld.create(1000, 1000)

local test = function()
    core:setentity(1, 10, 10, 20, 20)
    print(core:haveblock(1, 1, 9, 9), core:haveblock(10, 10, 10, 10))
    core:setentity(1)
    print(core:haveblock(10, 10, 20, 20), core:areaids(10, 10, 20, 20))
    print(core:haveblock(-1, -1, 0, 0))

    for i = 1, 100 do
        core:setentity(i, i, i, i, i)
    end
    print(#core:areaids(30, 30, 80, 80))
    local t = os.time()
    for i = 1, 100000 do
        core:areaids(30, 30, 80, 80)
    end
    print(os.time() - t)
end

local test = function()
    for i = 1, 10 do
        core:settroop(i * 10, 500, 500, 10, 10)
        core:setview(i * 10, 10, 10, 20, 20)
    end
    print(dump(core:troopview()))

    for i = 1, 1000 do
        core:settroop(i * 10, 500, 500, 10, 10)
        core:setview(i * 10, 10, 10, 20, 20)
    end
    local t = os.time()
    for i = 1, 35 do
        core:troopview()
    end
    print(os.time() - t)
end
test()