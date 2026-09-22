require "util"
local version = require "version"
local ldbtool = require "ldbtool"

local test = function()
    local init = function()
        version.init()
        for i = 1, 5000 do
            version.add("test" .. i)
        end
    end
    local revert = function()
        local ins = version.revert()
        ins.dump()
    end
    revert()
end

local test1 = function()
    local create = function()
        local ins = version.create_ins(1)
        for i = 1, 100 do
            version.add("test" .. i)
        end
        for i = 101, 200 do
            version.add("test" .. i - 100)
        end
        ins.dump()
        ins.del()
    end
    -- create()

    local revert = function()
        local ins = version.revert()
        ins.dump()
    end
    revert()
end
