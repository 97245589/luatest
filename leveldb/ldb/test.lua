require "util"
local ldb = require "lleveldb"
local db = require "db"

local p = ldb.create("db")
db.set_pdb(p)

local test = function()
    db.del("test")
    local t = os.time()
    for i = 1, 200000 do
        db.hset("test", i, i * 10)
    end
    print(os.time() - t, db.hget("test", 188888))
    db.del("test")
    print(#db.hgetall("test"))
end
test()

local test1 = function()
    db.del("test")
    db.hmset("test", 1, 10, 3, 30, 5, 50)
    print(dump(db.keys("*")))
    print(dump(db.hgetall("test")))
    print(db.hget("test", 5))
    print(dump(db.hmget("test", 5, 10, 1)))
    db.hdel("test", 3)
    print("after hdel 3")
    print(dump(db.hgetall("test")))
    db.del("test")
    print("after del test")
    print(dump(db.keys("*")))
    print(dump(db.hgetall("test")))
end

ldb.compact(p)
ldb.release(p)
