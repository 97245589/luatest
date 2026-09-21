require "util"
local ltrie = require "ltrie"

local test = function()
    local core = ltrie.create()
    for i = 1, 20 do
        core:insert(i, i);
    end
    core:erase(12)
    print(core:value(10), core:value(12))
    print(dump(core:prefix("1", 5)))
    print(dump(core:prefix("1", 0)))

    local bin = core:seri()
    local ncore = ltrie.create()
    ncore:deseri(bin)
    print(dump(ncore:prefix("", 0)))
end

local press = function()
    local core = ltrie.create()
    for i = 1, 100000 do
        core:insert(i, i);
    end
    print(dump(core:prefix("222", 5)))
    local bin = core:seri()
    print(#bin)

    local t = os.time()
    for i = 1, 100 do
        core:seri()
    end
    print("seri:", os.time() - t)

    t = os.time()
    for i = 1, 100 do
        local ncore = ltrie.create()
        ncore:deseri(bin)
    end
    print("deseri:", os.time() - t)
end
press()
