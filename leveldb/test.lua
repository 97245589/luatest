require "util"
local lleveldb = require "lleveldb"

local create = function()
    local db = lleveldb.create("db")
    return {
        realkeys = function()
            return db:realkeys()
        end,
        del = function(key)
            return db:del(key)
        end,
        keys = function(pat)
            return db:keys(pat)
        end,
        hset = function(key, hkey, v)
            db:hset(key, hkey, v)
        end,
        hget = function(key, hkey)
            return db:hget(key, hkey)
        end,
        hdel = function(key, hkey)
            return db:hdel(key, hkey)
        end,
        hgetall = function(key)
            return db:hgetall(key)
        end,
        hkeys = function(key)
            return db:hkeys(key)
        end,
        hmget = function(key, ...)
            return db:hmget(key, ...)
        end,
        hmset = function(key, ...)
            db:hmset(key, ...)
        end
    }
end

local test = function()
    local db = create()
    db.hmset(1, 2, 20, 5, 50, 9, 90)
    db.hset(1, 2, 30)
    db.hset(1, 7, 70)
    print("hgetall", dump(db.hgetall(1)))
    print("hkeys", dump(db.hkeys(1)))
    print("hmget", dump(db.hmget(1, 3, 7, 5)))
    print("hget", db.hget(1, 7), db.hget(1, 8))
    db.hdel(1, 5)
    print("after hdel hgetall", dump(db.hgetall(1)))
    print("realkeys", dump(db.realkeys()))
    db.hset(10, "hello", "world")
    print("keys", dump(db.keys("*")), dump(db.keys("*0")))
    db.del(1)
    print("after del", dump(db.keys("*")), dump(db.realkeys()))
end

local stress = function()
    local db = create()
    local t = os.time()
    for i = 1, 200000 do
        db.hset(i, "hello" .. i, "world" .. i)
    end
    print(os.time() - t)

    local ret
    local t = os.time()
    for i = 1, 200000 do
        ret = db.hgetall(i)
    end
    print(os.time() - t, dump(ret))
end
stress()
