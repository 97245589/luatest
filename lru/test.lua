local setmetatable = setmetatable
local next = next
local require = require
require "util"
local llru = require "llru"

local create_lru = function(num)
    local lru = llru.create(num)

    local obj = {{}}
    setmetatable(obj, {
        __index = function(tb, k)
            local v = tb[1][k]
            if v then
                lru:update(k)
            end
            return v
        end,
        __newindex = function(tb, k, v)
            if v ~= nil then
                local evict = lru:update(k)
                if evict then
                    tb[1][evict] = nil
                end
            else
                lru:del(k)
            end
            tb[1][k] = v
        end,
        __pairs = function(tb)
            return next, tb[1]
        end
    })
    return obj
end

local obj = create_lru(5)
for i = 1, 5 do
    obj[tostring(i)] = 1
end
local v = obj[tostring(3)]
local v = obj[tostring(10)]
obj[tostring(3)] = nil
for i = 6, 9 do
    obj[tostring(i)] = 1
end
print(dump(obj))
