require "util"
local lraft = require "lraft"

local rafts = {}

local cb_enum = {
    send_requestvote = 1,
    send_appendentries = 2,
    applylog = 3,
    persist_vote = 4,
    persist_term = 5,
    log_offer = 6,
    log_pop = 7
}

local create_raft = function(num, idx)
    local nodes = {}
    local ret = {}

    local cbs = {
        [cb_enum.send_requestvote] = function(id, bin)
            -- print(string.format("%s send requestvote %s", idx, id))
            rafts[id].requestvote(idx, bin)
        end,
        [cb_enum.send_appendentries] = function(id, heartbeatbin, entry, entrybin)
            -- print(string.format("%s send appendentries %s", idx, id), entry and "have entry " .. #entrybin)
            rafts[id].appendentries(idx, heartbeatbin, entry, entrybin)
        end,
        [cb_enum.applylog] = function(entryidx, entry)
            print("applylog", entryidx, entry)
        end,
        [cb_enum.persist_vote] = function(voteid)
            print("persist_vote", voteid)
        end,
        [cb_enum.persist_term] = function(term, vote)
            print("persist_term", term, vote)
        end,
        [cb_enum.log_offer] = function(entryidx, entry)
            print("log_offer", entryidx, entry)
        end,
        [cb_enum.log_pop] = function(entryidx, entry)
            print("log_pop", entryidx, entry)
        end
    }

    local core = lraft.create(cbs)
    for i = 1, num do
        local isme = i == idx
        local pnode = core:add_node(i, isme)
        nodes[i] = {
            isme = isme,
            pnode = pnode
        }
        -- print(pnode, isme)
    end

    ret.requestvote = function(id, bin)
        -- print(string.format("%s recv requestvote %s", idx, id), #bin)
        local pnode = nodes[id].pnode
        local resbin = core:requestvote(pnode, bin)
        rafts[id].requestvote_res(idx, resbin)
    end

    ret.requestvote_res = function(id, bin)
        print(string.format("%s recv requestvote_res %s", idx, id), #bin)
        local pnode = nodes[id].pnode
        core:requestvote_res(pnode, bin)
    end

    ret.appendentries = function(id, heartbeatbin, entry, entrybin)
        -- print(string.format("%s recv appendentries %s", idx, id))
        local pnode = nodes[id].pnode
        local resbin = core:appendentries(pnode, heartbeatbin, entry, entrybin)
        rafts[id].appendentries_res(idx, resbin)
    end

    ret.appendentries_res = function(id, bin)
        print(string.format("%s recv appendentries_res %s", idx, id))
        local pnode = nodes[id].pnode
        core:appendentries_res(pnode, bin)
    end

    ret.recv_entry = function(str)
        local leader, lid = core:get_leader()
        if not leader then
            print("no leader")
            return
        end
        if lid ~= idx then
            print("im not leader")
            return
        end
        local bin = core:recv_entry(str)
        print("entry_res_commited", core:entry_res_commited(bin))
    end

    ret.tick = function()
        core:tick()
    end
    return ret
end

for i = 1, 3 do
    rafts[i] = create_raft(3, i)
end

rafts[1].tick()
rafts[1].tick()
rafts[1].recv_entry("hello")
rafts[1].tick()
rafts[1].tick()
