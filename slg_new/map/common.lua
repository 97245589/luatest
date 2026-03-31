local require = require
local print, dump = print, dump
local math, pairs = math, pairs

local M = require "map.world"

M.new_monster = function(level)
    local entity = {
        type = "monster",
        level = 10
    }
    return entity
end

M.new_res = function(subtype, level)
    local entity = {
        type = "res",
        subtype = subtype,
        level = level
    }
    return entity
end

M.clear_type = {
    res = 1,
    monster = 1
}

M.get_troop_src = function()
    local ret = {}
    local troops = M.get_troops()
    for wid, troop in pairs(troops) do
        local src = troop.src
        if src then
            ret[src] = ret[src] or {}
            ret[src][wid] = 1
        end
    end
    return ret
end

M.get_dest_entities = function()
    local ret = {}
    local troops = M.get_troops()
    for wid, troop in pairs(troops) do
        local eid = troop.dwid
        ret[eid] = ret[eid] or {}
        ret[eid][wid] = 1
    end
    return ret
end

M.clear_area = function(blx, bly, len)
    local dest_entities = M.get_dest_entities()
    local area_entities = M.area_entities(blx, bly, len)

    for wid, entity in pairs(area_entities) do
        if M.clear_type[entity.type] then
            if not dest_entities[entity.wid] then
                M.del_entity(entity.wid)
            end
        end
    end
end

M.force_clean_are = function(blx, bly, len)
    local area_entities = M.area_entities(blx, bly, len)
    for wid, entity in pairs(area_entities) do
        M.del_entity(entity)
    end
end

local add_flush_entity = function(entity, blx, bly, len)
    local trx = blx + len - 1
    local try = bly + len - 1

    local i = 0
    while true do
        local x = math.random(blx, trx)
        local y = math.random(bly, try)
        if M.add_entity(x, y, 1, entity) then
            return
        end
        -- print("flush enter fail", blx, trx, bly, try, x, y)
        i = i + 1
        if i > 3 then
            return
        end
    end
end

local flush_mon = function(blx, bly, len, conf)
    if not conf then
        return
    end
    for _, v in pairs(conf) do
        for i = 1, v.num do
            local mon = M.new_monster(v.level)
            add_flush_entity(mon, blx, bly, len)
        end
    end
end

local flush_res = function(blx, bly, len, conf)
    if not conf then
        return
    end

    for _, v in pairs(conf) do
        for i = 1, v.num do
            local res = M.new_res(v.subtype, v.level)
            add_flush_entity(res, blx, bly, len)
        end
    end
end

-- monster = {{level= 5,num=10}}
-- res = {{subtype=1,level=3,num=10}}
M.flush_one_grid = function(blx, bly, len, conf)
    flush_mon(blx, bly, len, conf.monster)
    flush_res(blx, bly, len, conf.res)
end

M.flush = function(grid_size, conf)
    local world_len = M.get_len()
    local world_wid = M.get_wid()

    for i = 1, world_len / grid_size do
        for j = 1, world_wid / grid_size do
            M.flush_one_grid(grid_size * (i - 1), grid_size * (j - 1), grid_size, conf)
        end
    end
end

M.troop_arrive_type_cb = {}

M.troop_arrive = function(troop)
    --[[
    print("troop arrive", dump(troop))

    local dtype = troop.dtype
    local dwid = troop.dwid
    local entities = M.get_entities()
    local entity = entities[dwid]
    if entity then
        print("troop arrive entity", dump(entity))
    end

    local func = M.troop_arrive_type_cb[dtype]
    if func then
        func(troop, entity)
    end
    ]]
end

return M
