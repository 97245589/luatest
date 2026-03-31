require "util"
local lworld = require "lworld"

local function create_lworld()
    local world = lworld.create_lworld()
    return {
        init = function(len, wid)
            world:init(len, wid)
        end,
        add_entity = function(cx, cy, len, id)
            return world:add_entity(cx, cy, len, id)
        end,
        del_entity = function(cx, cy, len)
            return world:del_entity(cx, cy, len)
        end,
        area_entities = function(cx, cy, len)
            return world:area_entities(cx, cy, len)
        end,
        recover_troop = function(troop)
            world:recover_troop(troop)
        end,
        add_troop = function(id, tm, speed, path)
            world:add_troop(id, tm, speed, path)
        end,
        del_troop = function(id)
            world:del_troop(id)
        end,
        troops_move = function(tm)
            local arrive_arr = world:troops_move(tm)
            return arrive_arr
        end,
        troops_info = function()
            return world:troops_info()
        end,
        troop_watches = function()
            return world:troop_watches()
        end,
        add_watch = function(id, weigh, cx, cy)
            world:add_watch(id, weigh, cx, cy)
        end,
        del_watch = function(id)
            world:del_watch(id)
        end
    }
end

local test_troop_watches = function()
    local world = create_lworld()
    world.init(1000, 1000)

    --[[
    for i = 1, 10 do
        world.add_troop(i, 0, 1, {0, 0, 900, 900})
    end
    world.add_watch(1, 1, 500, 500)
    world.add_watch(2, 1, 500, 500)
    print(dump(world.troop_watches()))
    ]]

    for i = 1, 12000 do
        world.add_troop(i, 0, 1, {0, 0, 900, 900});
    end
    for i = 1, 4000 do
        world.add_watch(i, 1, 900, 900)
    end

    local t = os.time()
    for i = 1, 100 do
        world.troop_watches()
    end
    print(os.time() - t)
end

local test_watch = function()
    local world = create_lworld()
    world.init(1000, 1000)

    for i = 1, 20 do
        world.add_watch(i * 10, i, 10, 10)
    end
    print(dump(world.add_entity(6, 6, 2, 100)))
    print(dump(world.del_entity(6, 6, 2)))

    --[[
    for i = 1, 4000 do
        world.add_watch(i, i, 10, 10)
    end

    local t = os.time()
    for i = 1, 100000 do
        world.add_entity(6, 6, 1, 2)
        world.del_entity(6, 6, 1)
    end
    print(os.time() - t)
    ]]
end

local test_recover_troop = function()
    local world = create_lworld()
    world.init(1000, 1000)

    local t = os.time()
    for i = 1, 1000000 do
        local troop = {
            worldid = i,
            path = {0, 0, 900, 900},
            speed = 1,
            nowx = 1,
            nowy = 1,
            nowpos = 0,
            tm = i * 1000
        }
        world.recover_troop(troop)
    end
    print(os.time() - t)
end

local test_troop = function()
    local world = create_lworld()
    world.init(1000, 1000)
    --[[
    world.add_troop(200, 0, 3, {10, 0, 0, 0})
    world.add_troop(100, 0, 5, {1, 1, 2, 2, 4, 4, 6, 6, 10, 10})
    print(dump(world.troops_info()))
    for i = 1, 5 do
        local arr = world.troops_move(i * 1000)
        if arr then
            print("arrived", dump(arr))
        end
        print(dump(world.troops_info()))
    end
    ]]

    for i = 1, 12000 do
        world.add_troop(i, 0, 0.1, {1, 1, 900, 900})
    end

    local t = os.time()
    for i = 1, 2000 do
        world.troops_info()
    end
    print("world infos", os.time() - t)

    local t = os.time()
    for i = 1, 10000 do
        world.troops_move(i * 1000)
    end
    print("tick troop", os.time() - t)
end

local test_entites = function()
    local world = create_lworld()
    world.init(200, 200)

    for i = 1, 100 do
        world.add_entity(i, i, 1, i)
    end
    print(dump(world.area_entities(0, 0, 50)))

    local t = os.time()
    for i = 1, 100000 do
        world.area_entities(0, 0, 100)
    end
    print(os.time() - t)
end

local test_troop_grid = function()
    local world = create_lworld()
    world.init(1000, 1000)

    --[[
    world.add_troop(21, 0, 20, {0, 0, 200, 200})
    world.add_troop(20, 0, 20, {0, 0, 200, 200})
    world.troop_watches()
    for i = 1, 20 do
        print("tm ", i)
        world.troops_move(i * 1000)
        world.troop_watches()
    end
    ]]

    for i = 1, 12000 do
        world.add_troop(i, 0, 1, {0, 0, 500, 500})
    end

    local t = os.time()
    for i = 1, 1000 do
        world.troop_watches()
    end
    print(os.time() - t)
end
