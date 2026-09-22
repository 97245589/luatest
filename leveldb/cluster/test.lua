require "util"
local version = require "version"
local ldbtool = require "ldbtool"

-- execute("mkdir version")

local test = function ()
    version.sort_dump_files()
end
test()

local test = function()
    local init = function()
        version.init()
        for i = 1, 5000 do
            version.add("test" .. i)
        end
    end
    init()
    local revert = function()
        local ins = version.revert()
        ins.dump()
    end
end

local test = function()
    local create = function()
        local ins = version.create_ins(1)
        for i = 1, 100 do
            version.add("test" .. i)
        end
        for i = 101, 200 do
            version.add("test" .. i - 100)
        end
        ins.dump()
    end
    -- create()

    local revert = function()
        local ins = version.revert()
        ins.dump()
    end
end
