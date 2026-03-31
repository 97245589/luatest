require "util"
local pairs = pairs
local next = next

local attr_handle
local all_attr = function(player)
    local attrs = {}
    for _, func in pairs(attr_handle) do
        local fattrs = func(player)
        for id, val in pairs(fattrs) do
            local ov = attrs[id] or 0
            ov = ov + val
            attrs[id] = ov
        end
    end
    player.attrs = attrs
end
local add_diff = function(player, oattrs, nattrs)
    local pattrs = player.attrs
    local diff = {}
    for id, nv in pairs(nattrs) do
        local ov = oattrs[id] or 0
        oattrs[id] = nil
        if ov ~= nv then
            local pv = pattrs[id] or 0
            pv = pv + nv - ov
            pattrs[id] = pv
            diff[id] = pv
        end
    end
    for id, ov in pairs(oattrs) do
        local pv = pattrs[id] or 0
        pv = pv - ov
        pattrs[id] = pv
        diff[id] = pv
    end
    if next(diff) then
        return diff
    end
end
local diff_attr = function(player, attrtype, func)
    local oattrs = attr_handle[attrtype](player)
    func()
    local nattrs = attr_handle[attrtype](player)
    add_diff(player, oattrs, nattrs)
end

local test = function()
    local oattrs = {}
    local nattrs = {}
    for i = 1, 20 do
        oattrs[i] = 1
        nattrs[i + 10] = 1
    end
    attr_handle = {
        [1] = function(player)
            return oattrs
        end,
        [2] = function(player)
            return oattrs
        end
    }

    local player = {}
    all_attr(player)
    print("before diff", dump(player))
    local diff = add_diff(player, oattrs, nattrs)
    print("after diff", dump(player), dump(diff))
end

local test1 = function()
    attr_handle = {
        [1] = function(player)
            local attrs = {}
            for i = 1, 300 do
                attrs[i] = 1
            end
            return attrs
        end,
        [2] = function(player)
            local attrs = {}
            for i = 1, 300 do
                attrs[i] = 1
            end
            return attrs
        end
    }
    local player = {}
    all_attr(player)

    local t = os.time()
    local ret
    for i = 1, 100000 do
        ret = diff_attr(player, 1, function()
        end)
    end
    print(os.time() - t, ret)
    -- print(dump(player))
end
test1()
