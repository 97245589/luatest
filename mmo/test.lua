require "util"
local lworld = require "lworld"

local core = lworld.create(1000, 1000)

local astar = function()
    -- core:setblock(300, 300, 1)
    print(dump(core:astar(0, 0, 500, 500)))

    local t = os.time()
    for i = 1, 1000 do
        core:astar(0, 0, 500, 500)
    end
    print(os.time() - t)
end

local search = function()
    for i = 1, 200 do
        core:addactor(i, i, i, 1, 1, 1)
    end

    print(dump(core:search(100, true, 1, 5)))
    print(#core:aoiids(100))

    local t = os.time()
    for i = 1, 200000 do
        core:search(1, true, 1, 5)
    end
    print(os.time() - t)
end

local setpos = function()
    for i = 1, 200 do
        core:addactor(i, i, i, 1, 1, 1)
    end

    for i = 1, 30 do
        local adds, dels = core:setpos(1, i, i, 1, 1)
        print("pos", i, i)
        if adds then
            print("adds", dump(adds))
        end
        if dels then
            print("dels", dump(dels))
        end
    end
    print(core:dumpaoi())
end