require "util"
local lmap = require "lmap"

local march = function()
    local core = lmap.create(1024, 1024)
    local test = function()
        for i = 1, 10 do
            core:add_march(i, 1, 1, 100, 100)
        end
        local vid = 999999
        core:set_vision(vid, 50, 50, 60, 60)
        print(dump(core:vision_march(vid)))
    end

    local press = function()
        for i = 1, 5000 do
            core:add_march(i, 1, 1, 100, 100)
        end
        local vid = 999999
        core:set_vision(vid, 50, 50, 60, 60)
        print(#core:vision_march(vid))

        local t = os.time()
        for i = 1, 10000 do
            core:vision_march(vid)
        end
        print(os.time() - t)
    end
    press()
end
march()

local area_vid = function()
    local test = function()
        local core = lmap.create(1024, 1024)
        for i = 1, 3 do
            core:set_vision(i, 10, 10, 10, 10)
        end
        print(dump(core:area_vid(9, 9, 10, 10)))
    end
    test()
    local press = function()
        local core = lmap.create(1024, 1024)
        for i = 1, 5000 do
            core:set_vision(i, 15, 15, 20, 20)
        end
        -- print(#core:area_vid(7, 7, 12, 12))
        local t = os.time()
        for i = 1, 100000 do
            core:area_vid(7, 7, 12, 12)
        end
        print(os.time() - t)
    end
end

local area_eid = function()
    local core = lmap.create(1024, 1024)
    for x = 100, 200 do
        for y = 100, 200 do
            local id = x * 1000 + y
            core:add_entity(id, x, y, x, y)
        end
    end

    print(#core:area_eid(101, 101, 130, 130))

    local t = os.time()
    for i = 1, 100000 do
        core:area_eid(101, 101, 130, 130)
    end
    print(os.time() - t)
end

local test = function()
    local core = lmap.create(1024, 1024)
    for i = 1, 100 do
        core:add_entity(i, i, 0, i, 0)
    end
    print(dump(core:area_eid(0, 0, 10, 0)))
    print(core:area_obstacle(1024, 1024, 1024, 1024))
    print(core:area_obstacle(1, 1, 30, 30))
    print(core:area_obstacle(0, 0, 30, 30))
end
