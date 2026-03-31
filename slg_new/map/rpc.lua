local require = require
local cmds = require "common.service.cmds"

local mod_name, mod, world

local notify_entity = function(watches, entity, ope)
end

local notify_troop = function(ret)
end

cmds.start = function(name)
    mod_name = "map." .. name .. ".init"
    mod = require(mod_name)
    world = mod.init({
        notify_entity = notify_entity,
        notify_troop = notify_troop
    })
end

cmds.player_enter = function(playerid)

end

cmds.add_watch = function()

end

cmds.player_leave = function()

end
