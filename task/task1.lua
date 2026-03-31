require "util"

local enums = {
    unfinish = 1,
    finish = 2,

    player_level = 1,
    accu_test = 2,

    pet_lv_num = 3
}

local task_cfg = {
    [1] = {
        -- player level 3
        event = {1, 3}
    },
    [2] = {
        -- player level 5
        event = {1, 5}
    },
    [10] = {
        -- accumulation
        event = {2, 10}
    },
    [20] = {
        -- 2 pets whose lv is higher than 2
        event = {3, 2, 2}
    }
}
-- start ===============
local M = {
    enums = enums,
    task_mgrs = {}
}
local handle = {}

local count_task = function(player, task, tevent, add_num)
    local change
    local ep1 = tevent[1]
    if handle[ep1] then
        local bnum = task.num
        task.num = handle[ep1](player, tevent, task) or 0
        if bnum ~= task.num then
            change = true
        end
    else
        task.num = task.num + add_num
        if add_num ~= 0 then
            change = true
        end
    end

    local num = tevent[#tevent]
    if task.num >= num then
        task.num = num
        task.status = enums.finish
    end
    return change
end

M.init_task = function(player, task_obj, task_cfg)
    task_obj.marks = task_obj.marks or {}
    task_obj.tasks = task_obj.tasks or {}

    local tasks = task_obj.tasks
    local marks = task_obj.marks
    for taskid, tcfg in pairs(task_cfg) do
        local tevent = tcfg.event
        if tasks[taskid] then
            goto cont
        end
        local task = {
            id = taskid,
            num = 0,
            status = enums.unfinish
        }
        count_task(player, task, tevent, 0)
        if task.status == enums.unfinish then
            local emark = tevent[1]
            marks[emark] = marks[emark] or {}
            marks[emark][taskid] = 1
        end
        tasks[taskid] = task
        ::cont::
    end
end

M.count_task = function(player, task_obj, task_cfg, event)
    local emark = event[1]
    local marks = task_obj.marks
    local taskids = marks[emark]
    if not taskids or not next(taskids) then
        return
    end

    local arr = {}
    local add_num = event[#event]
    for taskid, _ in pairs(taskids) do
        local tevent = task_cfg[taskid].event
        local task = task_obj.tasks[taskid]
        local change = count_task(player, task, tevent, add_num)
        if change then
            table.insert(arr, taskid)
        end
        if task.status ~= enums.unfinish then
            marks[emark][taskid] = nil
            if not next(marks[emark]) then
                marks[emark] = nil
            end
        end
    end
    return next(arr) and arr
end

M.trigger_event = function(player, pevent)
    for name, mgr in pairs(M.task_mgrs) do
        mgr.trigger_event(player, pevent)
    end
end

M.add_taskmgr = function(name, mgr)
    M.task_mgrs[name] = mgr
end
M.del_taskmgr = function(name)
    M.task_mgrs[name] = nil
end
-- end ===================

handle[enums.player_level] = function(player, tevent, task)
    return player.level
end

handle[enums.pet_lv_num] = function(player, tevent, task)
    local pets = player.pets
    local elv = tevent[1]
    local num = 0
    for k, pet in pairs(pets) do
        if pet.level >= elv then
            num = num + 1
        end
    end
    return num
end

local player = {
    level = 3,
    task = {},
    pets = {
        [10] = {
            level = 5
        },
        [20] = {
            level = 3
        }
    }
}

local task_mgr = {
    trigger_event = function(player, pevent)
        M.count_task(player, player.task, task_cfg, pevent)
    end
}
M.add_taskmgr("test", task_mgr)

M.init_task(player, player.task, task_cfg)
print("player after init", dump(player))
player.level = 10
M.trigger_event(player, {enums.player_level, 10})
M.trigger_event(player, {enums.accu_test, 8})
print("after trigger", dump(player))
