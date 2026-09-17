require "util"
local ldb = require "lleveldb"
local db = require "db"

local p = ldb.create("db")
db.set_pdb(p)

local scan = function()
    for i = 1, 20 do
        db.hset(i, "test", i * 10)
    end

    --[[
    print(dump(db.keys("*")))
    print(dump(db.scan(0)))
    print(dump(db.scan(0, "match", "1*", "COUNT", 3)))
    ]]

    local tarveral = function(match, count, cb)
        local input = { 0 }
        if match then
            table.insert(input, "match")
            table.insert(input, match)
        end
        if count then
            table.insert(input, "count")
            table.insert(input, count)
        end
        local cursor = 0
        while cursor ~= "0" do
            input[1] = cursor
            local ret = db.scan(table.unpack(input))
            cursor = ret[1]
            if not cb(ret[2]) then
                return
            end
        end
    end

    tarveral("1*", 5, function(arr)
        print("===", dump(arr))
        return true
    end)

    for i = 1, 20 do
        db.del(i)
    end
end
scan()

local hscan = function()
    for i = 1, 50 do
        db.hset("test", i, i * 10)
    end

    --[[
    print(dump(db.hscan("test", 0)))
    print(dump(db.hscan("test", 0, "match", "*3*")))
    print(dump(db.hscan("test", 0, "MATCH", "*5", "COUNT", 3)))
    ]]


    local traversal = function(key, match, count, cb)
        local input = { key, 0 }
        if match then
            table.insert(input, "match")
            table.insert(input, match)
        end
        if count then
            table.insert(input, "count")
            table.insert(input, count)
        end
        local cursor = 0
        while cursor ~= "0" do
            input[2] = cursor
            local ret = db.hscan(table.unpack(input))
            cursor = ret[1]
            if not cb(ret[2]) then
                return
            end
        end
    end

    traversal("test", "*3*", nil, function(arr)
        print("traversal", dump(arr))
        return true
    end)

    db.del("test")
end

local test = function()
    db.del("test")
    db.hmset("test", 1, 10, 3, 30, 5, 50)
    print(dump(db.hgetall("test")))
    print(db.hget("test", 5))
    print(dump(db.hmget("test", 5, 10, 1)))
    db.hdel("test", 3)
    print("after hdel 3")
    print(dump(db.hgetall("test")))
    db.del("test")
    print("after del test")
    print(dump(db.hgetall("test")))
end

ldb.compact(p)
ldb.release(p)
