require "util"
local pairs = pairs

local attr_handle = {}
local all_attr = function(player)
    local oattrs = player.attrs
    local nattrs = {}

    for _, func in pairs(attr_handle) do
        func(player, nattrs)
    end

    player.attrs = nattrs
    if not oattrs then
        return nattrs
    end
    local ret = {}
    for id, nv in pairs(nattrs) do
        local ov = oattrs[id]
        if nv ~= ov then
            ret[id] = nv
        end
        oattrs[id] = nil
    end
    for id, ov in pairs(oattrs) do
        ret[id] = 0
    end
    return ret
end

local test1 = function()
    local ofunc = function(player, attrs)
        for i = 1, 300 do
            local v = attrs[i] or 0
            v = v + 1
            attrs[i] = v
        end
    end
    for i = 1, 10 do
        attr_handle[i] = ofunc
    end
    local player = {}
    local ret
    all_attr(player)

    local t = os.time()
    for i = 1, 100000 do
        ret = all_attr(player)
    end
    print(os.time() - t)

    print(dump(ret))
end
test1()

local test = function()
    local ofunc = function(player, attrs)
        for i = 1, 20 do
            local v = attrs[i] or 0
            v = v + 1
            attrs[i] = v
        end
    end
    local nfunc = function(player, attrs)
        for i = 11, 30 do
            local v = attrs[i] or 0
            v = v + 1
            attrs[i] = v
        end
    end

    attr_handle[1] = ofunc
    local player = {}
    local ret = all_attr(player)
    print(dump(ret))
    attr_handle[1] = nfunc
    local ret = all_attr(player)
    print(dump(ret))
end
