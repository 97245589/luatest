require "util"
local ljps = require "ljps"

local test = function()
    local map, core
    local setblock = function()
        for i = 1, #map do
            for j = 1, #map[i] do
                local v = map[i][j]
                if v ~= 0 then
                    core:setblock(j - 1, #map - i, v)
                end
            end
        end
    end
    local formatpath = function(arr)
        if not arr then
            return
        end
        local ret = {}
        for i = 1, #arr, 2 do
            local x = arr[i]
            local y = arr[i + 1]
            ret[x] = ret[x] or {}
            ret[x][y] = 1
        end
        local wid = #map
        local len = #map[1]
        for y = wid - 1, 0, -1 do
            local str = ""
            for x = 0, len - 1 do
                local v = ret[x] and ret[x][y]
                if not v then
                    str = str .. "* "
                else
                    str = str .. "p "
                end
            end
            print(str)
        end
    end
    map = {
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 1, 1, 0 },
        { 0, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 0 }
    }
    core = ljps.create(#map, #map[1])
    setblock()
    core:jpcache()
    print(core:dumpcache(1, 0))
    formatpath(core:find(0, 0, 5, 0))
end

local test = function()
    local core = ljps.create(1000, 1000)
    core:setblock(300, 300, 1)
    core:jpcache()
    print(dump(core:find(1, 1, 999, 999)))

    local t = os.time()
    for i = 1, 20 do
        core:jpcache()
    end
    print(os.time() - t)

    local t = os.time()
    for i = 1, 10000 do
        core:find(1, 1, 999, 999)
    end
    print(os.time() - t)
end
test()