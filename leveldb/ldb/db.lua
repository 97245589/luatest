local ldb = require "lgame.leveldb"
local pdb

local SEPARATOR = string.char(0xff)

local M = {}

local scan = function(cursor, patt, count)
    patt = patt or "*"
    count = count or 10
    local start = ""
    local end_ = SEPARATOR
    if type(cursor) == "string" then
        start = cursor
    end

    local arr = ldb.scan(pdb, start, end_, patt, count, 1)
    local ret = {}
    if #arr > count then
        ret[1] = table.remove(arr)
    else
        ret[1] = "0"
    end
    ret[2] = arr
    return ret
end

M.scan = function(cursor, ...)
    local arr = table.pack(...)
    local match, count
    for i = 1, #arr, 2 do
        local name = string.lower(arr[i])
        local v = arr[i + 1]
        if name == "match" then
            match = v
        elseif name == "count" then
            count = v
        end
    end
    return scan(cursor, match, count)
end

local traversal = function(patt, count, cb)
    local cursor = 0
    while cursor ~= "0" do
        local ret = scan(cursor, patt, count)
        -- print(dump(ret))
        cursor = ret[1]
        if not cb(ret[2]) then
            return
        end
    end
end
M.traversal = traversal

local hscan = function(key, cursor, patt, count)
    patt = patt or "*"
    count = count or 10
    local start = key .. SEPARATOR
    local end_ = key .. SEPARATOR .. SEPARATOR
    if type(cursor) == "string" then
        start = cursor
    end
    local arr = ldb.scan(pdb, start, end_, patt, count)
    local ret = {}
    if #arr > 2 * count then
        ret[1] = table.remove(arr)
    else
        ret[1] = "0"
    end
    ret[2] = arr
    return ret
end

M.hscan = function(key, cursor, ...)
    local match, count
    local arr = table.pack(...)
    if #arr % 2 ~= 0 then
        print("hscan err", #arr)
        return
    end
    for i = 1, #arr, 2 do
        local name = string.lower(arr[i])
        local v = arr[i + 1]
        if name == "match" then
            match = v
        elseif name == "count" then
            count = v
        end
    end
    return hscan(key, cursor, match, count)
end

local htraversal = function(key, patt, count, cb)
    local cursor = 0
    while cursor ~= "0" do
        local ret = hscan(key, cursor, patt, count)
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
    traversal(patt, nil, function(arr)
        table.move(arr, 1, #arr, #ret + 1, ret)
        return true
    end)
    return ret
end

M.del = function(key)
    htraversal(key, nil, nil, function(arr)
        for i = 1, #arr, 2 do
            local field = arr[i]
            M.hdel(key, field)
        end
        return true
    end)
end

M.hgetall = function(key)
    local ret = {}
    htraversal(key, nil, nil, function(arr)
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
    local rawkey = key .. SEPARATOR .. field
    ldb.put(pdb, rawkey, val)
end

M.hdel = function(key, ...)
    local arr = table.pack(...)
    for idx, field in ipairs(arr) do
        local rawkey = key .. SEPARATOR .. field
        ldb.del(pdb, rawkey)
    end
end

M.hget = function(key, field)
    local rawkey = key .. SEPARATOR .. field
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
