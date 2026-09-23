local mode, paddr = ...
local skynet = require "skynet"

if mode == "child" then
    local start = require "server.service.service"
    local cmds = require "server.func.cmd"
    local dbimpl = require "server.func.dbimpl"

    dbimpl.set_writecb(function(...)
        skynet.send("version", "lua", "dbwrite", ...)
    end)

    cmds.set_pdb = function(pdb)
        dbimpl.set_pdb(pdb)
    end

    cmds.ope = function(cmd, ...)
        return dbimpl[cmd](...)
    end

    start(function()
    end)
else
    local ldb = require "lgame.leveldb"
    local cfg = require "server.db.cfg"
    local M = {}
    local addrs = {}
    local path = "run/db/" .. skynet.getenv("server_name")
    local pdb = ldb.create(path, 1024 * 1024 * 16)

    local self = skynet.self()
    for i = 1, 5 do
        local addr = skynet.newservice("server/db/proxy/db", "child", self)
        skynet.send(addr, "lua", "set_pdb", pdb)
        table.insert(addrs, addr)
    end

    local read_cmds = cfg.read_cmds
    local write_cmds = cfg.write_cmds
    local IDX = 2
    M.ope = function(cmd, ...)
        if write_cmds[cmd] then
            skynet.send(addrs[1], "lua", "ope", cmd, ...)
        elseif read_cmds[cmd] then
            local addr = addrs[IDX]
            local ret = skynet.call(addr, "lua", "ope", cmd, ...)
            IDX = IDX + 1
            if IDX > #addrs then
                IDX = 2
            end
            return ret
        else
            print("db ope err", cmd, ...)
        end
    end

    M.get_pdb = function()
        return pdb
    end

    return M
end
