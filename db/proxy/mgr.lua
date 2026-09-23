local skynet = require "skynet"
local cluster = require "skynet.cluster"
local cmds = require "server.func.cmd"
local slot = require "server.db.proxy.slot"
local cfg = require "server.db.cfg"
local db = require "server.db.proxy.db"

local read_cmds = cfg.read_cmds
local write_cmds = cfg.write_cmds
cmds.ope = function(cmd, key, ...)
    local isread = read_cmds[cmd]
    local iswrite = write_cmds[cmd]
    if not isread and not iswrite then
        print("err cluster ope", cmd, key, ...)
        return
    end
    local group = slot.group_by_key(key)
    local ok, master = skynet.call("group", "lua", "master_bygroup", group, key)
    if ok then
        return db.ope(cmd, key, ...)
    elseif master then
        return cluster.call(master, "proxy", "ope", cmd, key, ...)
    end
end

cmds.get_pdb = function()
    return db.get_pdb()
end
