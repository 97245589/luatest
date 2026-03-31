local require, print, dump = require, print, dump
local skynet = require "skynet"
local world = require "map.common"
local math = math

local test_flush = function()
    --[[
    world.flush(10, {
        monster = {{
            level = 5,
            num = 5
        }}
    })

    local entities = world.get_entities()
    local map = world.get_map()
    print(#entities)
    print(dump(entities))
    print(dump(map))

    world.del_entity(5)
    print(dump(entities))
    print(dump(map))

    world.clear_area(10, 0, 10)
    print(dump(entities))
    print(dump(map))
    ]]

    --[[
    local t = skynet.now()
    world.flush(10, {
        mon = {{
            level = 5,
            num = 20
        }}
    })
    print("mass flush:", skynet.now() - t, #world.get_entities())
    ]]
end

local test_troop = function()
    --[[
    world.add_entity(18, 18, 2, {
        type = "test"
    })

    world.add_troop(0, 0, 18, 18, 3, {})
    skynet.timeout(500, function()
        print("===== reset troop grids")
        world.reset_troop_grids()
    end)
    ]]

    --[[
    local troop = {}
    world.add_troop(0, 0, 9, 0, 2, troop)
    skynet.timeout(400, function()
        print("---------- delete troop")
        world.del_troop(troop)
    end)
    ]]

    world.add_troop(0, 0, 0, 200, 50, {})

    skynet.sleep(1)
    while true do
        print(dump(world.get_all_troopinfo()))
        print("=================")
        skynet.sleep(100)
    end

    --[[
    local t = skynet.now()
    for i = 1, 10000 do
        world.add_troop(0, 0, 990, 990, 20, {})
    end
    print(skynet.now() - t)
    ]]
end

local test_watch = function()
    world.add_entity(6, 6, 1, {})
    world.add_entity(14, 14, 1, {})
    world.add_entity(6, 7, 1, {})
    world.add_entity(7, 6, 1, {})
    world.add_entity(6, 14, 1, {})
    world.add_entity(6, 15, 1, {})
    world.add_entity(5, 6, 1, {})

    local ret = world.add_watch("test1", 10, 10)
    print(dump(ret))
    world.add_watch("test2", 5, 5)
    print(dump(world.get_all_watchinfo()))

    world.del_entity_bypos(6, 6)
    world.del_entity_bypos(6, 15)
    world.del_entity_bypos(5, 6)
    world.add_entity(8, 8, 1, {})
    world.del_watch("test1")
    world.del_watch("test2")
    world.del_entity_bypos(8, 8)
    print(dump(world.get_all_watchinfo()))

    --[[
    local t = skynet.now()
    for i = 1, 4000 do
        world.add_watch("w" .. i, math.random(6, 14), math.random(6, 14), math.random(1, 3))
    end
    for i = 1, 5000 do
        world.add_entity(10, 10, 1, {})
        world.del_entity_bypos(10, 10)
    end
    print(skynet.now() - t)
    ]]
end

local troop_watch = function()

    world.add_troop(0, 29, 40, 29, 3, {})

    local _, watch_troops = world.add_watch("w1", 20, 20)
    print("watch troops", dump(watch_troops))
    skynet.timeout(200, function()
        world.add_troop(16, 0, 16, 50, 3, {})
    end)

    --[[
    local t = skynet.now()
    for i = 1, 12000 do
        world.add_troop(0, 29, 40, 29, 3, {})
    end

    local watchnum = 4000
    for i = 1, watchnum do
        world.add_watch("w" .. i, 20, 20, 1)
    end
    print(skynet.now() - t)
    ]]
end

local ret = {}

ret.init = function(args)
    world.init_world({
        increid = 0,
        troops = {},
        entitys = {},
        cb = {
            init = nil,
            troop_arrive = function(troop)
                print("troop arrive", dump(troop))
            end,
            notify_entity = function(watches, entity, ope)
                print("notify entity", dump(watches), dump(entity), ope)
            end,
            notify_troop = function(ret)
                print("watch troop change ===", dump(ret))
            end
        }
        -- world_len = 20,
        -- world_wid = 20
    })

    return world
end

return ret
