require "util"
local lworld = require "lworld"

local core = lworld.create(1000, 1000)

for i = 1, 1000 do
    core:setblock(i, i, i, i, i * 10)
end

print(#core:areaids(10, 10, 60, 60))

local t = os.time()
for i = 1, 1000000 do
    core:areaids(10, 10, 60, 60)
end
print(os.time() - t)
