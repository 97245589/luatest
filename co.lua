local db = function(cmd, ...)
    return coroutine.yield(cmd, ...)
end

local co = coroutine.create(function()
    db("set", "hello", "world")
    print(db("get", "hello"))
end)

local simpledb = {
    __data = {},
    get = function(self, k)
        return self.__data[k]
    end,
    set = function(self, k, v)
        self.__data[k] = v
    end
}

local ok, cmd, p1, p2 = coroutine.resume(co)
-- print("step1", ok, cmd, p1, p2)
local ret = simpledb[cmd](simpledb, p1, p2)
local ok, cmd, p1, p2 = coroutine.resume(co, ret)
-- print("step2", ok, cmd, p1, p2)
local ret = simpledb[cmd](simpledb, p1)
coroutine.resume(co, ret)
