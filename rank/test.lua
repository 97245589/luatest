require "util"
local lrank = require "lrank"

local test = function()
    local rank = lrank.create(10)

    for i = 1, 20 do
        rank:add(i % 10, i + 100, 0)
    end

    print(dump(rank:info(3, 7)))
    print(dump(rank:info(1, 20)))
    print(rank:get_order(20))
end

local stress = function()
    local random = math.random
    local rank = lrank.create(1000)

    local t = os.time()
    for i = 1, 1000000 do
        rank:add(i % 2000, i, i)
    end
    print(os.time() - t)

    local t = os.time()
    local info
    for i = 1, 100000 do
        info = rank:info(500, 600)
    end
    print(os.time() - t, #rank:info(1, 9999))

    local t = os.time()
    local order
    for i = 1, 5000000 do
        order = rank:get_order(1100)
    end
    print(os.time() - t, order)
end
stress()
