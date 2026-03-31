require "util"
local table = table
local next = next
local type = type
local print = print
local dump = dump
local clone = clone

local updates = {}
local deletes = {}
local print_info = function()
    print(dump(updates, "updates"), dump(deletes, "deletes"))
    print("=============")
end

local fill
fill = function(pobj, uobj, dobj, path, deltype, depth)
    local k = path[depth]
    if depth == #path then
        if deltype then
            uobj[k] = deltype
        else
            uobj[k] = clone(pobj[k])
        end
        if type(dobj) == "table" then
            dobj[k] = nil
        end
        return
    end
    if not uobj[k] then
        local v = {}
        if k == pobj[k].id then
            v.id = k
        end
        uobj[k] = v
    end
    local dobj
    if type(dobj) == "table" then
        if type(dobj[k]) == "table" then
            dobjk = dobj[k]
        else
            dobj[k] = nil
        end
    end
    fill(pobj[k], uobj[k], dobjk, path, deltype, depth + 1)

    if type(dobjk) == "table" then
        local idk, idv = next(dobjk)
        local otherk = next(dobjk, idk)
        -- print("===", k, idk, idv, otherk)
        if not idk then
            dobj[k] = nil
        elseif idk == "id" and idv == k and not otherk then
            dobj[k] = nil
        end
    end
end

local update = function(player, path, deltype)
    local id = player.id
    table.insert(path, 1, id)

    local rp = { [id] = player }
    if deltype then
        fill(rp, deletes, updates, path, deltype, 1)
    else
        fill(rp, updates, deletes, path, deltype, 1)
    end
end

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
    local t = os.time()
    for i = 1, 1000000 do
        update(player, { "map", 200 })
        -- update(player, { "map", 200, "num" })
    end
    print(os.time() - t)
end
test()

local test = function()
    update(player, { "obj", "map", 10, "val" })
    print_info()
    update(player, { "obj", "map", 10 }, { id = 10 })
    print_info()
    update(player, { "obj", "map" }, {})
    print_info()
end

local test = function()
    update(player, { "map", 100, "obj", "val" })
    print_info()
    update(player, { "map", 100 }, { id = 100 })
    print_info()
    update(player, { "map", 100 })
    print_info()
    update(player, { "map", 100, "obj" }, {})
    print_info()
end

local test = function()
    -- player.map[100] = nil
    update(player, { "map", 100 }, { id = 100 })
    update(player, { "map", 200 }, { id = 200 })
    print_info()
    update(player, { "map", 100 })
    -- update(player, { "map", 200 })
    print_info()
end

local test = function()
    update(player, { "obj", "id" }, 0)
    print_info()
    update(player, { "obj", "val" })
    update(player, { "obj", "id" })
    print_info()
    update(player, { "obj", "id" }, 0)
    print_info()
end
