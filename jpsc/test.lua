require "util"
local ljps = require "ljps"

local create_jps = function()
    local jps = ljps.create_ljps()
    return {
        init_map = function(len, wid)
            jps:init_map(len, wid)
        end,
        set_block = function(blx, bly, trx, try, v)
            jps:set_block(blx, bly, trx, try, v)
        end,
        get_cache = function(dx, dy)
            return jps:get_cache(dx, dy)
        end,
        set_cache = function()
            jps:set_cache()
        end,
        block_cache = function(blx, bly, trx, try)
            jps:block_cache(blx, bly, trx, try)
        end,
        path = function(sx, sy, ex, ey, quick)
            return jps:path(sx, sy, ex, ey, quick)
        end
    }
end

local format_path = function(arr)
    if not arr then
        return
    end
    local str = ""
    for i = 1, #arr, 2 do
        str = str .. arr[i] .. "," .. arr[i + 1] .. " "
    end
    print(str)
end

local map_tool = function(jps)
    local map = [[
    0 0 0 0 0 
    0 0 0 0 0 
    0 0 0 0 0 
    0 1 1 1 0 
    0 0 0 0 0  
    ]]

    local gen_map = function(str)
        local ret = {}
        for w in string.gmatch(map, "([^\n]+)\n") do
            local arr = {}
            for n in string.gmatch(w, "[0-9]") do
                table.insert(arr, math.tointeger(n))
            end
            if next(arr) then
                table.insert(ret, 1, arr)
            end
        end
        return ret
    end

    local ret = gen_map(map)
    local wid = #ret
    local len = #ret[1]
    jps.init_map(len, wid)
    for y, arr in ipairs(ret) do
        for x, v in ipairs(arr) do
            if v ~= 0 then
                jps.set_block(x - 1, y - 1, x - 1, y - 1, 1)
            end
        end
    end
end

local test1 = function()
    local jps = create_jps()
    map_tool(jps)
    jps.set_cache()
    print(jps.get_cache(1, 0))
    format_path(jps.path(0, 1, 4, 1))
end

local test2 = function()
    local jps = create_jps()
    jps.init_map(1000, 1000)
    jps.set_cache()
    format_path(jps.path(0, 0, 500, 999))

    local t = os.time()
    for i = 1, 10000 do
        local ret = jps.path(0, 0, 999, 999)
    end
    print(os.time() - t)
end

local test3 = function()
    local jps = create_jps()
    jps.init_map(1500, 1500)

    local t = os.time()
    for i = 1, 10 do
        jps.set_cache()
    end
    print(os.time() - t)

    t = os.time()
    for i = 1, 5000 do
        jps.block_cache(100, 100, 102, 102)
    end
    print(os.time() - t)
end

local test4 = function()
    local jps = create_jps()
    jps.init_map(30, 30)
    jps.set_cache()
    print(jps.get_cache(1, 0))
    jps.set_block(10, 10, 11, 11, 1)
    jps.block_cache(10, 10, 11, 11)
    print(jps.get_cache(1, 0))
    jps.set_block(10, 10, 11, 11)
    jps.block_cache(10, 10, 11, 11)
    print(jps.get_cache(1, 0))
end
