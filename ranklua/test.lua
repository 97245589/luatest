require "util"
local rank_m = require "rank"

local rank = {
    arr = {},
    map = {},
    num = 1000
}

local t = os.time()
for i = 1, 100000 do
    rank_m.add(rank, {
        id = i % 2000,
        score = i,
        tm = i
    })
end
print(os.time() - t, #rank.arr)

local t = os.time()
local ret = {}
for i = 1, 50000 do
    ret = rank_m.info(rank, 1, 9999)
end
print(os.time() - t, #ret)

local t = os.time()
local ret
for i = 1, 500000 do
    ret = rank_m.get_order(rank, 1010)
end
print(os.time() - t, ret)
