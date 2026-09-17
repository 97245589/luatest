local ldb = require "lleveldb"
local pdb

local SPLIT = string.char(0xff)

local M = {}

M.scan = function(cursor, patt, count)
    patt = patt or "*"
    count = count or 10
    local start = ""
    local end_ = SPLIT
    if type(cursor) == "string" then
        start = cursor
    end
    local arr = ldb.scan(pdb, start, end_, patt, count, 1)
    local ret = {}
    if #arr > count then
        ret[1] = table.remove(arr)
    else
        ret[1] = 0
    end
    ret[2] = arr
    return ret
end

local traversal = function(patt, count, cb)
    local cursor
    while cursor ~= 0 do
        local ret = M.scan(cursor, patt, count)
        cursor = ret[1]
        if not cb(ret[2]) then
            return
        end
    end
end
M.traversal = traversal

M.hscan = function(key, cursor, patt, count)
    patt = patt or "*"
    count = count or 10
    local start
    if type(cursor) == "string" then
        start = cursor
    else
        start = key .. SPLIT
    end
    local end_ = key .. SPLIT .. SPLIT
    local arr = ldb.scan(pdb, start, end_, patt, count)
    local ret = {}
    if #arr > 2 * count then
        ret[1] = table.remove(arr)
    else
        ret[1] = 0
    end
    ret[2] = arr
    return ret
end

local htraversal = function(key, patt, count, cb)
    local cursor
    while cursor ~= 0 do
        local ret = M.hscan(key, cursor, patt, count)
        cursor = ret[1]
        if not cb(ret[2]) then
            return
        end
    end
end
M.htraversal = traversal

M.keys = function(patt)
    if not patt then
        print("keys no patt")
        return
    end
    local ret = {}
    traversal(patt, 3, function(arr)
        table.move(arr, 1, #arr, #ret + 1, ret)
        return true
    end)
    return ret
end

M.del = function(key)
    htraversal(key, nil, nil, function(arr)
        for i = 1, #arr, 2 do
            local field = arr[i]
            ldb.del(pdb, key .. SPLIT .. field)
        end
        return true
    end)
end

M.hgetall = function(key)
    local ret = {}
    htraversal(key, "*", 10, function(arr)
        table.move(arr, 1, #arr, #ret + 1, ret)
        return true
    end)
    return ret
end

M.hmset = function(key, ...)
    local arr = table.pack(...)
    if #arr % 2 ~= 0 then
        error("db hmset params err")
        return
    end
    for i = 1, #arr, 2 do
        M.hset(key, arr[i], arr[i + 1])
    end
end

M.hset = function(key, field, val)
    local rawkey = key .. SPLIT .. field
    ldb.put(pdb, rawkey, val)
end

M.hdel = function(key, ...)
    local arr = table.pack(...)
    for idx, field in ipairs(arr) do
        local rawkey = key .. SPLIT .. field
        ldb.del(pdb, rawkey)
    end
end

M.hget = function(key, field)
    local rawkey = key .. SPLIT .. field
    return ldb.get(pdb, rawkey)
end

M.hmget = function(key, ...)
    local arr = table.pack(...)
    local ret = {}
    for idx, field in ipairs(arr) do
        local val = M.hget(key, field)
        if val then
            ret[idx] = val
        end
    end
    return ret
end

M.compact = function()
    ldb.compact(pdb)
end

M.set_pdb = function(p)
    pdb = p
end

return M
