local skynet = require "skynet"

local server_name = skynet.getenv("server_name")
local exec_str = string.format("mkdir run/db/%s_version", server_name)
print(exec_str)

local M = {}

local version = 0

M.add = function()

end

M.get_version = function()
    return version
end

return M
