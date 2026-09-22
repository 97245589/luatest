require "util"

local test = function()
    local ldbtool = require "ldbtool"
    local core = ldbtool.create_version()

    for i = 1, 1000 do
        core:add("test" .. i, i)
    end
    print(dump(core:version_after(100, 10)))
    print(dump(core:version_after(980)))
    print("===", core:version_after(1000))
end
test()

local press = function()
    local ldbtool = require "ldbtool"
    local core = ldbtool.create_version()

    local t = os.time()
    for i = 1, 1000000 do
        core:add("test" .. i, i)
    end
    print(os.time() - t)
    print(dump(core:version_after(999990)))
end

local test = function()
    local ldbtool = require "ldbtool"
    local core = ldbtool.create_version()

    for i = 1, 10 do
        core:add("test" .. i, i)
    end
    for i = 1, 10 do
        core:add("test" .. i, i * 10)
    end
    print(core:size())
    print(core:dump())

    local ncore = ldbtool.create_version()
    local bin = core:seri()
    ncore:deseri(bin)
    print(ncore:dump())
end
