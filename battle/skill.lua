local require = require
local load = load
local table = table
local pairs = pairs
local ipairs = ipairs
local next = next
local string = string
local math = math

local skillcfg = {
    [101] = {
        targ = { "enemy" },
        action = "damage(fattr(src,ATK)*2-fattr(targ,DEF))",
    },
    [201] = {
        action = "addbuff(1) buffattr({[31]=10,[42]=10})"
    },
    [301] = {
        targ = { "enemy" },
        action = "addbuff(5) buffattr({[51]=-5}) local v=fattr(src,ATK)*10 buff_roundend(function() damage(v) end)"
    },
    [401] = {
        action = "local v=fattr(src,ATK) addbuff(10) buff_roundend(function() addhp(v) end)"
    },
    [501] = {
        action = "addbuff(6) buffevent(ESKILL, function(skillid) addhp(100) end)",
    }
}

local M = {
    HP = 10,
    MP = 20,
    ATK = 30,
    DEF = 40,
    SPEED = 50,

    ESKILL = 10,
    EATK = 20,
    EATKED = 21,
    battle = nil,
    targ = nil,
    src = nil,
    buff = nil
}
for id, cfg in pairs(skillcfg) do
    if cfg.action then
        cfg.action = load(cfg.action, "skill" .. id, "bt", M)
    end
end

M.rand = math.random

local targ_handle = {
    enemy = function()
        local battle = M.battle
        local atkorder = battle.atkorder
        local src = M.src
        local scamp = src.spot // 100
        for _, unit in ipairs(atkorder) do
            if unit.spot // 100 ~= scamp then
                return { unit }
            end
        end
    end,
    allteam = function()
    end,
}
M.get_targs = function(arr)
    -- print("targs", dump(arr))
    local func = targ_handle[arr[1]]
    if not func then
        return
    end
    return func(table.unpack(arr, 2, #arr))
end

M.fattr = function(unit, k)
    local attrs = unit.attrs
    local base = attrs[k] or 0
    local add = attrs[k + 1] or 0
    local per = attrs[k + 2] or 0
    return (base + add) * (1 + per)
end

M.addbuff = function(bufftid)
    local targ = M.targ
    local src = M.src
    local tbuff = targ.buff
    tbuff.idx = tbuff.idx + 1
    local buffid = tbuff.idx
    if tbuff.idx >= 20000 then
        tbuff.idx = 0
    end
    local newbuff = {
        id = buffid,
        tid = bufftid,
        endtm = 1,
        -- src = src,
        attrs = nil,
        event = nil
    }
    local buffs = tbuff.buffs
    buffs[buffid] = newbuff
    M.buff = newbuff
    local timer = M.battle.timer
    timer:add(targ.spot, buffid, newbuff.endtm)
end

M.removebuff = function(unit, buffid)
    unit.roundend[buffid] = nil
    local buffs = unit.buff.buffs
    local buff = buffs[buffid]
    buffs[buffid] = nil
    if not buff then
        return
    end
    -- print("remove buff===", unit.spot, buffid)
    if buff.attrs then
        local uattrs = unit.attrs
        for k, v in pairs(buff.attrs) do
            local uv = uattrs[k] or 0
            uattrs[k] = uv - v
        end
    end
    if buff.event then
        -- print("remove event buff", buff.event)
        local eventfunc = unit.event
        eventfunc[buff.event][buffid] = nil
    end
end

M.buffattr = function(attrs)
    local buff = M.buff
    if not buff then
        return
    end
    local targ = M.targ
    print("buffattrs", targ.spot, dump(attrs))
    buff.attrs = attrs
    local tattrs = targ.attrs
    for k, v in pairs(attrs) do
        local nv = tattrs[k] or 0
        tattrs[k] = nv + v
    end
end

M.buff_roundend = function(func)
    local buff = M.buff
    if not buff then
        return
    end
    local targ = M.targ
    local roundend = targ.roundend
    roundend[buff.id] = func
end

M.roundend = function(unit)
    M.targ = unit
    for _, func in pairs(unit.roundend) do
        func()
    end
    M.targ = nil
end

M.buffevent = function(event, func)
    local buff = M.buff
    if not buff then
        return
    end
    buff.event = event
    local targ = M.targ
    local eventfunc = targ.event
    eventfunc[event] = eventfunc[event] or {}
    eventfunc[event][buff.id] = func
end

M.triggerevent = function(unit, event, ...)
    M.targ = unit
    local eventfunc = unit.event
    if eventfunc[event] then
        for buffid, func in pairs(eventfunc[event]) do
            func(...)
        end
    end
    M.targ = nil
end

M.useskill = function(src, skillid)
    local cfg = skillcfg[skillid]
    if not cfg then
        return
    end
    local targarr = cfg.targ
    local action = cfg.action
    if not action then
        return
    end
    local targs
    M.src = src
    if not targarr then
        targs = { src }
    else
        targs = M.get_targs(targarr)
    end
    if targs then
        for _, targ in ipairs(targs) do
            M.targ = targ
            action()
        end
    end
    M.triggerevent(src, M.ESKILL, skillid)
    M.src = nil
    M.targ = nil
    M.buff = nil
end

M.damage = function(val)
    local targ = M.targ
    -- print("damage", targ.spot, val)
end

M.addhp = function(val)
    local targ = M.targ
    -- print("addhp", targ.spot, val)
end

return M
