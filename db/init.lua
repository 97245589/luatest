local start = require "server.service.service"
local skynet = require "skynet"

start(function()
    local group_addr = skynet.newservice("server/db/group/init")
    local sc = require "server.service.cluster"
    sc.set_diff_cb(function(upd, del)
        skynet.send(group_addr, "lua", "cluster_diff", upd, del)
    end)
    skynet.newservice("server/db/proxy/init")
    skynet.newservice("server/db/version/init")
end, "cluster")
