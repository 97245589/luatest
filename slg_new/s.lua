require "util"
local math, print = math, print

local troop = {
    path = {},
    nowx = 1,
    nowy = 0,
    pathpos = 1
}

local check_arrive = function(troop)
    if troop.pathpos * 2 >= #troop.path then
        return true
    end
    return false
end

local get_next_pos = function(troop)
    local path = troop.path
    local next_pos = troop.pathpos + 1
    if 2 * next_pos > #path then
        return
    end
    return path[2 * next_pos - 1], path[2 * next_pos]
end

local troop_move_dis
troop_move_dis = function(troop, move_dis)
    if check_arrive(troop) then
        return
    end
    if move_dis <= 0 then
        return
    end
    local nowx, nowy = troop.nowx, troop.nowy
    local nextx, nexty = get_next_pos(troop)
    local dx, dy = nextx - nowx, nexty - nowy
    local np_dis = math.sqrt(dx ^ 2 + dy ^ 2)
    if move_dis < np_dis then
        troop.nowx = nowx + move_dis * dx / np_dis
        troop.nowy = nowy + move_dis * dy / np_dis
    else
        troop.pathpos = troop.pathpos + 1
        if check_arrive(troop) then
            troop.nowx = troop.path[#troop.path - 1]
            troop.nowy = troop.path[#troop.path]
        else
            troop_move_dis(troop, move_dis - np_dis)
        end
    end
    -- print(troop.nowx, troop.nowy, troop.pathpos)
end

local test = function()
    local troop = {
        path = {0, 0, 10, 0, 990, 1000},
        nowx = 0,
        nowy = 0,
        pathpos = 0
    }
    for i = 1, 100 do
        troop_move_dis(troop, 1)
    end
end

local t = os.time()
for i = 1, 12000 do
    test()
end
print(os.time() - t)
