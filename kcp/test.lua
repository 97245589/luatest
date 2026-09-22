require "util"
local lkcp = require "lkcp"

local send_core, recv_core

recv_core = lkcp.create(function(str)
    print("recv_cb", str)
end)

send_core = lkcp.create(function(str)
    print("send_cb", str)
    recv_core:input(str)
end)
send_core:send("hello")
send_core:update(10)

recv_core:update(10)
print("recv", recv_core:recv())
