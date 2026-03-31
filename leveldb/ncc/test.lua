require "util"
local printt = function(t)
    print(dump(t))
end

local lleveldb = require "lleveldb"
local pdb = lleveldb.create("db")

lleveldb.hmset(pdb, "test", "hello", "world", 10, 11, 20, 21)
printt(lleveldb.hmget(pdb, "test", 20, 100, "hello"))
printt(lleveldb.hgetall(pdb, "test"))

lleveldb.hdel(pdb, "test", 20)
printt(lleveldb.hgetall(pdb, "test"))
printt(lleveldb.keys(pdb, "*"))
lleveldb.del(pdb, "test")
printt(lleveldb.keys(pdb, "*"))
lleveldb.compact(pdb)

lleveldb.release(pdb)
