require "util"
local ltimer = require "ltimer"

local create_timer = function(func)
    local core = ltimer.create()
    local M = {}

    M.add = function(id, tm, mark)
        core:add(id, tm, mark)
    end

    M.del_mark = function(id, mark)
        core:del_mark(id, mark)
    end

    M.del_id = function(id)
        core:del_id(id)
    end

    M.expire = function(tm)
        local arr = core:expire(tm)
        for i = 1, #arr, 2 do
            local id = arr[i]
            local mark = arr[i + 1]
            func(id, mark)
        end
    end

    M.dump = function()
        print(core:dump())
    end

    return M
end

local test = function()
    local timer = create_timer(function(id, mark)
        print(id, mark)
    end)
    local t = os.time()
    for i = 1, 100000 do
        timer.add(tostring(i % 10000), i % 10000, tostring(i % 50000))
    end
    print(os.time() - t)

    local t = os.time()
    for i = 1, 1000000 do
        timer.expire(1)
    end
    print(os.time() - t)
end

local test1 = function()
    local timer = create_timer(function(id, mark)
        print("expire", id, mark)
    end)
    for i = 11, 15 do
        timer.add(tostring(1), i, tostring(i))
        timer.add(tostring(2), i + 10, tostring(i + 10))
    end
    timer.add(tostring(1), 30, tostring(11))
    timer.del_mark(tostring(1), tostring(12))
    timer.expire(25)
    timer.dump()

end
