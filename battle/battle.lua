local require = require
local table = table
local pairs = pairs
local ipairs = ipairs
local skill = require "skill"
local skillcfg = skill.skillcfg

local battle

local M = {}

local roundend = function()
    local atkorder = battle.atkorder
    for idx, unit in ipairs(atkorder) do
        local ubuff = unit.buff
        for buffid, cb in pairs(ubuff.roundend) do
            cb(unit)
        end
    end
end

local round = function()
    battle.round = battle.round + 1
    battle.atkorder = {}
    local atkorder = battle.atkorder
    for spot, unit in pairs(battle.spots) do
        if unit.attrs[skill.HP + 5] <= 0 then
            goto cont
        end
        table.insert(atkorder, unit)
        ::cont::
    end
    for idx, unit in ipairs(atkorder) do
        local scfg = skillcfg[501]
        local targs = scfg.targ(unit)
        if not targs then
            goto cont
        end
        for _, targ in ipairs(targs) do
            scfg.action(unit, targ)
        end
        skill.triggerevent(unit, skill.ESKILL, 201)
        ::cont::
    end
    roundend()
    battle.atkorder = nil
end

local init = function(atk, def)
    local initunit = function(unit, spot)
        unit.spot = spot
        unit.attrs[skill.HP + 5] = skill.fattr(unit, skill.HP)
        unit.buff = {
            idx = 0,
            buffs = {},
            roundend = {},
            event = {},
        }
    end
    battle = {}
    local spots = {}
    for k, unit in pairs(atk) do
        local spot = 100 + k
        initunit(unit, spot)
        spots[spot] = unit
    end
    for k, unit in pairs(def) do
        local spot = 200 + k
        initunit(unit, spot)

        spots[spot] = unit
    end
    battle.spots = spots
    battle.round = 0
    battle.report = {}
    skill.battle = battle
end
M.start = function(atk, def)
    init(atk, def)
    for i = 1, 1 do
        round()
    end
    battle = nil
end

return M
