require "util"
local table = table
local pairs = pairs
local ipairs = ipairs
local next = next
local type = type
local print = print
local dump = dump
local clone = clone

local updfunc
updfunc = function(id, info)
    print("upd", id, dump(info))
end
local delfunc
delfunc = function(id, info)
    print("del", id, dump(info))
end
local updates = {}
local deletes = {}

local tick = function()
    for id, upd in pairs(updates) do
        updfunc(id, upd)
        updates[id] = nil
    end
    for id, del in pairs(deletes) do
        delfunc(id, del)
        deletes[id] = nil
    end
end

local update = function(player, path, deltype)
    local id = player.id
    local udobj
    if deltype then
        local upd = updates[id]
        if upd then
            updates[id] = nil
            updfunc(id, upd)
        end
        deletes[id] = deletes[id] or {}
        udobj = deletes[id]
    else
        local del = deletes[id]
        if del then
            deletes[id] = nil
            delfunc(id, del)
        end
        updates[id] = updates[id] or {}
        udobj = updates[id]
    end

    local pobj = player
    local k = table.remove(path)
    for _, pk in ipairs(path) do
        if not udobj[pk] then
            local v = {}
            if pk == pobj[pk].id then
                v.id = pk
            end
            udobj[pk] = v
        end
        udobj = udobj[pk]
        pobj = pobj[pk]
    end
    if deltype then
        udobj[k] = deltype
    else
        udobj[k] = pobj[k]
    end
end

local test = function()
    local fill_upd
    fill_upd = function(pobj, uobj)
        for k, v in pairs(uobj) do
            if type(v) == "table" and k == v.id then
                v.id = nil
            end
            local pv = pobj[k]
            if type(pv) == "table" and type(v) == "table" then
                fill_upd(pv, v)
            else
                pobj[k] = v
            end
        end
    end

    local fill_del
    fill_del = function(pobj, dobj)
        for k, v in pairs(dobj) do
            if type(v) == "table" and k == v.id then
                v.id = nil
            end
            local pv = pobj[k]
            if type(v) ~= "table" or not next(v) then
                pobj[k] = nil
            elseif type(pv) == "table" then
                fill_del(pv, v)
            end
        end
    end
    local player = {
        id = 10,
        obj = {
            id = 10, val = 10,
        },
        map = {
            [100] = { id = 100, num = 100 },
        }
    }

    local player1 = clone(player)
    updfunc = function(id, upd)
        print("upd", dump(upd))
        fill_upd(player1, upd)
    end
    delfunc = function(id, del)
        print("del", dump(del))
        fill_del(player1, del)
    end

    update(player, { "obj", "val" }, 0)
    update(player, { "map", 100, "num" }, 0)

    -- player.obj.val = 20
    -- update(player, { "obj", "val" })
    -- player.map[100].num = 1000
    -- update(player, { "map", 100, "num" })
    tick()
    print(dump(player1))
end
test()


local player = {
    id = 10,
    obj = {
        id = 10,
        val = 10,
        map = { [10] = { id = 10, val = 10, val1 = 10 } }
    },
    map = {
        [100] = { id = 100, num = 100, obj = { id = 100, val = 100 } },
        [200] = { id = 200, num = 200 }
    }
}
local test = function()
    update(player, { "map", 100, "obj", "val" })
    update(player, { "map", 100, "obj" }, {})
    update(player, { "map", 100 })
    update(player, { "map", 100 }, { id = 100 })
    tick()
end

local test = function()
    updfunc = function() end
    delfunc = function() end
    local t = os.time()
    for i = 1, 1000000 do
        update(player, { "map", 100, "obj", "val" })
        update(player, { "map", 100, "obj" }, {})
        update(player, { "map", 100 })
        update(player, { "map", 100 }, { id = 100 })
        tick()
    end
    print(os.time() - t)
end
