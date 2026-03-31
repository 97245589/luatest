local table, pairs = table, pairs
local lworld = require "lworld"

local core = lworld.create_lworld()

local world_len, world_wid
local worldid

local entities
local troops

local watches = {}
local watch_troops = {}

local M = {}

M.tm = function()
    return os.time() * 1000
end

M.gen_worldid = function()
    worldid = (worldid + 1) & 0xfffffff
    if (worldid <= 0) then
        worldid = 1
    end
    return worldid
end

M.get_entities = function()
    return entities
end

M.init = function(args)
    worldid = args.worldid or 0
    entities = args.entities or {}
    troops = args.troops or {}
    world_len = args.len or 1000
    world_wid = args.wid or 1000

    core:init(world_len, world_wid)
end

M.area_entity = function(cx, cy, len)
    return core:area_entities(cx, cy, len)
end

M.add_entity = function(cx, cy, len, entity)
    if (M.area_entities(cx, cy, len)) then
        return
    end
    entity.worldid = entity.worldid or M.gen_worldid()
    entity.cx = cx
    entity.cy = cy
    entity.len = len
    entities[entity.worldid] = entity
    local ws = core:add_entity(cx, cy, len, entity.id)
    return true
end

M.del_entity = function(worldid)
    local entity = entities[worldid]
    if not entity then
        return
    end
    local cx = entity.cx
    local cy = entity.cy
    local len = entity.len
    if (not M.area_entities(cx, cy, len)) then
        return
    end
    local ws = core:del_entity(cx, cy, len)
end

M.recover_troop = function(troop)
    troops[troop.worldid] = troop
    core:recover_troop(troop)
end

M.add_troop = function(speed, path, troop)
    if speed <= 0 then
        return
    end
    troop.worldid = troop.worldid or M.gen_worldid()

    local tm = M.tm()
    local ret = core:add_troop(troop.worldid, tm, speed, path)
    if not ret then
        return
    end
    troop.speed = speed
    troop.path = path
    troop.tm = tm
    troop.nowpos = 0
    troop.nowx = path[1]
    troop.nowy = path[2]

    troops[troop.worldid] = troop
end

M.del_troop = function(worldid)
    troops[worldid] = nil
    core:del_troop(worldid)
end

M.add_watch = function(src, weigh, cx, cy)
    if not watches[src] then
        watches[src] = M.gen_worldid()
    end
    local worldid = watches[src]
    watch_troops[worldid] = {}
    core:add_watch(worldid, weigh, cx, cy)
    return worldid
end

M.del_watch = function(src)
    local worldid = watches[src]
    if worldid then
        watches[src] = nil
        watch_troops[worldid] = nil
        core:del_watch(worldid)
    end
end

M.troops_move = function(tm)
    local arrive_arr = core:troops_move(tm)
    if not arrive_arr then
        return
    end
end

M.troops_info = function(tm)
    local arr = core:troops_info()
    for i = 1, #arr, 4 do
        local worldid = arr[i]
        local nowx = arr[i + 1]
        local nowy = arr[i + 2]
        local nowpos = arr[i + 3]

        local troop = troops[worldid]
        if troop then
            troop.nowx = nowx
            troop.nowy = nowy
            troop.nowpos = nowpos
            troop.tm = tm
        end
    end
end

M.troop_watches = function()
    local obj = {}
    local compare = function(id, before, now)
        for twid in pairs(now) do
            if before[twid] then
                before[twid] = nil
            else
                obj[id] = obj[id] or {}
                obj[id].add = obj[id].add or {}
                obj[id].add[twid] = troops[twid]
            end
        end
        for twid in pairs(before) do
            obj[id] = obj[id] or {}
            obj[id].del = obj[id].del or {}
            obj[id].del[twid] = 1
        end
    end

    local infos = core:troop_watches()
    for worldid, info in pairs(infos) do
        local before = watch_troops[worldid]
        watch_troops[worldid] = info
        compare(before, info)
    end
end

M.tick = function(tm)
    tm = tm or M.tm()
    M.troops_move(tm)
    M.troops_info(tm)
    M.troop_watches()
end

return M
