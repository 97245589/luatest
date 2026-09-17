require "util"
local ldb = require "lleveldb"
local db = require "db"

local p = ldb.create("db")
db.set_pdb(p)

local test = function()
    for i = 1, 10 do
        db.hmset(i, "data", i)
    end

    print(dump(db.keys("*")))

    for i = 1, 10 do
        db.del(i)
    end
end
test()

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

ldb.release(p)
