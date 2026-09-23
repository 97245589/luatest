local skynet = require "skynet"
local cluster = require "skynet.cluster"
local cmds = require "server.func.cmd"

local server_name = skynet.getenv("server_name")
local myid = tonumber(skynet.getenv("server_id"))
local mygroupid = myid // 10

local master
local group_master = {}
local groups = {
    [mygroupid] = { [server_name] = 1 }
}

local parse_servername = function(name)
    local c2 = string.sub(name, 1, 2)
    if "db" ~= c2 then
        return
    end
    local id = tonumber(string.sub(3, -1))
    return id, id // 10
end

local mygroup_servers = function()
    local mygroup = groups[mygroupid]
    local arr = {}
    for k in pairs(mygroup) do
        table.insert(arr, k)
    end
    table.sort(arr)
    if arr[1] ~= server_name then
        return
    end
    return arr
end

local send_master = function(name)
    if name == server_name then
        return
    end
    cluster.send(name, "group", "group_master", mygroupid, master)
end
local notify_master = function(upd)
    if not master then
        return
    end
    local myservers = mygroup_servers()
    if not myservers then
        return
    end
    for _, name in ipairs(upd) do
        send_master(name)
    end
end

local select_master = function()
    if master then
        return
    end
    local servers = mygroup_servers()
    if not servers then
        return
    end
    if 1 == #servers then
        master = server_name
    else
        master = server_name
    end

    for gid, gservers in pairs(groups) do
        for name in pairs(gservers) do
            send_master(name)
        end
    end
end

local add_db_servers = function(servers)
    local dservers = {}
    for name in pairs(servers) do
        local id, group = parse_servername(name)
        if not id then
            goto cont
        end
        if group == mygroupid then
            master = nil
        end
        table.insert(dservers, name)
        groups[group] = groups[group] or {}
        groups[group][name] = 1
        ::cont::
    end
    notify_master(dservers)
end

local del_db_servers = function(servers)
    for name in pairs(servers) do
        local id, group = parse_servername(name)
        if not id then
            goto cont
        end
        if name == master then
            master = nil
        end
        local gservers = groups[group]
        gservers[name] = nil
        if not next(gservers) then
            print("err no group!!!", group)
        end
        ::cont::
    end
end

cmds.cluster_diff = function(upd, del)
    add_db_servers(upd)
    del_db_servers(del)
    print("dbserves groups:", dump(groups))
    select_master()
end

cmds.group_master = function(group, mname)
    group_master[group] = mname
    if group == mygroupid then
        master = mname
    end
    print(string.format("%s recv group master %s", server_name, mname))
end

cmds.master_bygroup = function(group)
    if group == mygroupid and master == server_name then
        return true
    end
    return false, group_master[group]
end

cmds.get_master = function()
    if server_name == master then
        return
    end
    return master
end
