local start = require "server.service.service"

start(function()
    require "server.db.version.mgr"
end, "version")
