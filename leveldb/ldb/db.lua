local ldb = require "lleveldb"
local pdb

local SPLIT = string.char(0xff)

local M = {}

local traversal = function(start, end_, patt, count, cb)
    patt = patt or "*"
    count = count or 10

    while true do
        local arr = ldb.scan(pdb, start, end_, patt, count)
        if #arr >= count * 2 + 2 then
            table.remove(arr)
            start = table.remove(arr)
            if not cb(arr) then
                return
            end
        else
            cb(arr)
            return
        end
    end
end

local parse_rawkey = function(rawkey)
    return string.match(rawkey, "^(.-)" .. SPLIT .. "(.*)$")
end
local htraversal = function(key, patt, count, cb)
    local start = key .. SPLIT
    local end_ = start .. SPLIT
    traversal(start, end_, patt, count, function(arr)
        for i = 1, #arr, 2 do
            local rawkey = arr[i]
            local k, field = parse_rawkey(rawkey)
            arr[i] = field
        end
        if cb(arr) then
            return true
        end
    end)
end
M.htraversal = htraversal

M.keys = function(patt)
    if not patt then
        print("keys no patt")
        return
    end
    local ret = {}
    traversal("", SPLIT, patt, nil, function(arr)
        for i = 1, #arr, 2 do
            local k, field = parse_rawkey(arr[i])
            if ret[#ret] ~= k then
                table.insert(ret, k)
            end
        end
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
    htraversal(key, nil, 3, function(arr)
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
    for idx, field in pairs(arr) do
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

M.set_pdb = function(p)
    pdb = p
end

return M
