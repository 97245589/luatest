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
        targ = "enemy(src, 1)",
        action = "damage(targ, fattr(src,ATK)*2-fattr(targ,DEF), src)"
    },
    [201] = {
        targ = "me(src)",
        action = "buffattr(targ, src, 1, {[ATK+2]=1})"
    },
    [301] = {
        targ = "enemy(src, 1)",
        action = "local v=fattr(src,ATK) buff_roundend(targ,src,1,function(targ) damage(targ,v) end)"
    },
    [401] = {
        targ = "me(src)",
        action = "buff_roundend(targ,src,1,function(targ) addhp(targ,200) end)"
    },
    [501] = {
        targ = "me(src)",
        action = "buffevent(targ,src,1,ESKILL,function(targ, skillid) addhp(targ,100) end)",
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
    skillcfg = skillcfg
}
local loadcfg = function()
    --[[
    for i = 1, 10000 do
        skillcfg[i] = {
            targ = "me(src)",
            action = "buffevent(targ,src,1,ESKILL,function(targ, skillid) addhp(targ,100) end)",
        }
    end
    ]]
    local starg = "return function(src) return %s end"
    local saction = "return function(src, targ) %s end"
    for id, cfg in pairs(skillcfg) do
        if not cfg.targ or not cfg.action then
            goto cont
        end
        local targf = string.format(starg, cfg.targ)
        cfg.targ = load(targf, "skilltarg" .. id, "bt", M)()
        local actionf = string.format(saction, cfg.action)
        cfg.action = load(actionf, "skillaction" .. id, "bt", M)()
        ::cont::
    end
    -- print("after loadskillcfg", collectgarbage("count"))
end
loadcfg()

M.rand = function(...)
    return math.random(...)
end

M.enemy = function(src, num)
    local atkorder = M.battle.atkorder
    local srct = src.spot // 100
    local arr = {}
    for _, unit in ipairs(atkorder) do
        if srct ~= unit.spot // 100 then
            table.insert(arr, unit)
            return arr
        end
    end
    return arr
end
M.me = function(src)
    return { src }
end

M.fattr = function(unit, k)
    local attrs = unit.attrs
    local base = attrs[k] or 0
    local add = attrs[k + 1] or 0
    local per = attrs[k + 2] or 0
    return (base + add) * (1 + per)
end

M.addbuff = function(targ, bufftid, src)
    local tbuff = targ.buff
    tbuff.idx = tbuff.idx + 1
    local buffid = tbuff.idx
    if tbuff.idx >= 20000 then
        tbuff.idx = 0
    end
    local newbuff = {
        id = buffid,
        tid = bufftid,
        endtm = 10,
    }
    local buffs = tbuff.buffs
    buffs[buffid] = newbuff
    return newbuff
end

M.removebuff = function(unit, buffid)
    local ubuff = unit.buff
    ubuff.roundend[buffid] = nil
    local buffs = ubuff.buffs
    local buff = buffs[buffid]
    buffs[buffid] = nil
    if not buff then
        return
    end
    if buff.attrs then
        local uattrs = unit.attrs
        for k, v in pairs(buff.attrs) do
            local uv = uattrs[k] or 0
            uv = uv - v
            uattrs[k] = uv
        end
    end
    if buff.event then
        local event = buff.event
        local ubevent = ubuff.event
        ubevent[event][buffid] = nil
        if not next(ubevent[event]) then
            ubevent[event] = nil
        end
    end
end

M.buffattr = function(targ, src, bufftid, attrs)
    local buff = M.addbuff(targ, bufftid, src)
    if not buff then
        return
    end
    buff.attrs = attrs
    local tattrs = targ.attrs
    for k, v in pairs(attrs) do
        local tv = tattrs[k] or 0
        tv = tv + v
        tattrs[k] = tv
    end
    -- print("buffattr", targ.spot, dump(targ))
end

M.buff_roundend = function(targ, src, bufftid, func)
    local buff = M.addbuff(targ, bufftid, src)
    if not buff then
        return
    end
    local tbuff = targ.buff
    tbuff.roundend[buff.id] = func
end

M.buffevent = function(targ, src, bufftid, event, func)
    local buff = M.addbuff(targ, bufftid, src)
    if not buff then
        return
    end
    buff.event = event
    local tbuff = targ.buff
    local tbevent = tbuff.event
    tbevent[event] = tbevent[event] or {}
    tbevent[event][buff.id] = func
end

M.triggerevent = function(unit, event, ...)
    local ubevent = unit.buff.event
    if ubevent[event] then
        for buffid, cb in pairs(ubevent[event]) do
            cb(unit, ...)
        end
    end
end

M.damage = function(targ, val, src)
    print("damage", targ.spot, val)
end

M.addhp = function(targ, val, src)
    print("addhp", targ.spot, val)
end

return M
