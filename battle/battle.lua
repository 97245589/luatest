local require = require
local table = table
local pairs = pairs
local ipairs = ipairs
local skill = require "skill"
local stimer = require "stimer"

local battle

local M = {}

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
        skill.useskill(unit, 101)
    end
    for idx, unit in ipairs(atkorder) do
        skill.roundend(unit)
    end
    local expire = battle.timer:expire(battle.round)
    if expire then
        for i = 1, #expire, 2 do
            local spot = expire[i]
            local buffid = expire[i + 1]
            skill.removebuff(battle.spots[spot], buffid)
        end
    end
    battle.atkorder = nil
end

local init = function(atk, def)
    local initunit = function(unit, spot)
        unit.spot = spot
        unit.attrs[skill.HP + 5] = skill.fattr(unit, skill.HP)
        unit.buff = {
            idx = 0,
            buffs = {},
        }
        unit.roundend = {}
        unit.event = {}
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
    battle.timer = stimer.create()
    skill.battle = battle
end
M.start = function(atk, def)
    init(atk, def)
    for i = 1, 10 do
        round()
    end
    battle = nil
end

return M
