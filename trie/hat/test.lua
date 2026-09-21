require "util"
local ltrie = require "ltrie"

local test = function()
    local core = ltrie.create()
    for i = 1, 20 do
        core:set(i, i)
    end
    local bin = core:seri()
    print(#bin)
    local ncore = ltrie.create()
    ncore:deseri(bin)
    print(dump(ncore:prefix_range("")))
end
test()

local test = function()
    local core = ltrie.create()
    for i = 1, 50 do
        core:set(i, i)
    end
    print(dump(core:prefix_range(2)))
    print(core:size())
    core:erase_prefix(1)
    print(core:size())
    print(dump(core:prefix_range("")))
end

local test = function()
    local core = ltrie.create()
    core:set(1, 10)
    print(core:get(1))
    core:set(1, 100)
    print(core:get(1))
    core:erase(1)
    print(core:get(1))
end
