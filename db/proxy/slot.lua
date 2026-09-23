local ldbtool = require "lgame.dbtool"
local cfg = require "server.db.cfg"
local toolf = require "server.func.tool"

local crc16 = toolf.crc16
local M = {}

local create_slot_group = function(slot_group)
    local core = ldbtool.create_slot(slot_group)
    return function(id)
        return core:find_group(id)
    end
end

M.reload = function(slot_group)
end

local SLOT_NUM = cfg.SLOT_NUM
local ins = create_slot_group(cfg.slot_group)
M.group_by_key = function(key)
    local slot = crc16(key) % SLOT_NUM
    return ins(slot)
end

return M
