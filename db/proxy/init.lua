local start = require "server.service.service"

start(function()
    require "server.db.proxy.mgr"
end, "proxy")
