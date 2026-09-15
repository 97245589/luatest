require "util"
local db = require "lleveldb"

local hscan = function()
    local traversal = function(pdb, key, match, count, cb)
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
            local rarr = db.hscan(pdb, table.unpack(input))
            cursor = rarr[1]
            if not cb(rarr[2]) then
                return
            end
        end
    end

    local pdb = db.create("db")

    for i = 1, 20 do
        db.hset(pdb, "test", i, i * 10)
    end

    traversal(pdb, "test", "*5", 8, function(arr)
        print(dump(arr))
        return true
    end)

    -- print(dump(db.hscan(pdb, "test", 0)))
    -- print(dump(db.hscan(pdb, "test", 0, "count", 5)))
    -- print(dump(db.hscan(pdb, "test", 0, "match", "*5", "COUNT", 15)))

    db.del(pdb, "test")
    db.compact(pdb)
    db.release(pdb)
end
hscan()

local test = function()
    local pdb = db.create("db")

    db.hmset(pdb, "test", 1, 10, 2, 20, 5, 50)
    print(dump(db.keys(pdb, "*")))
    print(db.hget(pdb, "test", 1))
    print(dump(db.hmget(pdb, "test", 1, 3, 2)))
    print(dump(db.hgetall(pdb, "test")))
    db.hdel(pdb, "test", 2, 3, 1)
    print(dump(db.hgetall(pdb, "test")))
    print(dump(db.hkeys(pdb, "test")))
    db.del(pdb, "test")
    print(dump(db.keys(pdb, "*")))
    print(dump(db.hgetall(pdb, "test")))

    db.compact(pdb)
    db.release(pdb)
end
