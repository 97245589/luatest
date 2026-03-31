local require = require
local skynet = require "skynet"
local table, ipairs, pairs, next = table, ipairs, pairs, next
local print, dump = print, dump
local math = math

local format = string.format

local WATCH_LEN = 5
local TROOP_GRID = 5
local world_len, world_wid = 1000, 1000

local increid, troops, entities, cb

local map
local watches, watches_src = {}, {}
local troop_grids = {}
local src_watch_troops = {}

local tm = function()
    return skynet.time() * 1000
end

local troop_grid_pos = function(x, y)
    return math.floor(x / TROOP_GRID), math.floor(y / TROOP_GRID)
end

local gen_worldid = function()
    increid = (increid + 1) % 0xffffffff
    return increid
end

local correct_x = function(x)
    if x < 0 then
        return 0
    end
    if x >= world_len then
        return world_len - 1
    end
    return x
end

local correct_y = function(y)
    if y < 0 then
        return 0
    end
    if y >= world_wid then
        return world_wid - 1
    end
    return y
end

local check_pos = function(blx, bly, len)
    if blx < 0 or blx >= world_len then
        return false
    end
    local trx = blx + len - 1
    if trx < 0 or trx >= world_len then
        return false
    end
    if bly < 0 or bly >= world_wid then
        return false
    end
    local try = bly + len - 1
    if try < 0 or try >= world_wid then
        return false
    end
    return true
end

local M = {}

M.init_world = function(obj)
    if obj.world_len then
        world_len = obj.world_len
    end
    if obj.world_wid then
        world_wid = obj.world_wid
    end
    cb = obj.cb
    if cb.init then
        cb.init()
    end

    increid = obj.increid or 0
    troops = obj.troops or {}
    entities = obj.entities or {}

    M.reset_map()
    M.reset_troop_grids()
end

M.area_entities = function(blx, bly, len)
    blx = correct_x(blx)
    bly = correct_y(bly)
    local trx = correct_x(blx + len - 1)
    local try = correct_y(bly + len - 1)

    local ret = {}
    for i = blx, trx do
        for j = bly, try do
            if map[i] and map[i][j] then
                local wid = map[i][j]
                ret[wid] = entities[wid]
            end
        end
    end
    return next(ret) and ret
end

local fill_map = function(entity)
    local blx = entity.blx
    local bly = entity.bly
    local len = entity.len
    local wid = entity.wid
    local trx = blx + len - 1
    local try = bly + len - 1
    for i = blx, trx do
        for j = bly, try do
            map[i] = map[i] or {}
            map[i][j] = wid
        end
    end
end

M.reset_map = function()
    map = {}
    for wid, entity in pairs(entities) do
        fill_map(entity)
    end
end

M.add_entity = function(blx, bly, len, entity)
    if len < 1 then
        return
    end
    if not check_pos(blx, bly, len) then
        return
    end
    local ret = M.area_entities(blx, bly, len)
    if ret then
        return
    end

    local wid = gen_worldid()
    entity.wid = wid
    entity.blx = blx
    entity.bly = bly
    entity.len = len
    entities[wid] = entity

    fill_map(entity)
    M.notify_watch(entity, "add")
    return true
end

local unfill_map = function(entity)
    local blx = entity.blx
    local bly = entity.bly
    local len = entity.len
    local trx = blx + len - 1
    local try = bly + len - 1
    for i = blx, trx do
        for j = bly, try do
            if map[i] then
                map[i][j] = nil
            end
            if not next(map[i]) then
                map[i] = nil
            end
        end
    end
end
M.del_entity = function(wid)
    if not wid then
        return
    end
    local entity = entities[wid]
    entities[wid] = nil
    unfill_map(entity)
    M.notify_watch(entity, "del")
end

M.del_entity_bypos = function(x, y)
    local wid = map[x][y]
    M.del_entity(wid)
end

M.add_troop = function(sx, sy, dx, dy, v, troop)
    if not troop then
        print("troop enter err no troop")
        return
    end
    if sx == dx and sy == dy then
        return
    end
    if not check_pos(sx, sy, 1) or not check_pos(dx, dy, 1) then
        return
    end
    local wid = gen_worldid()

    local dirx = dx - sx
    local diry = dy - sy
    local dis = math.sqrt(dirx ^ 2 + diry ^ 2)
    local unitx = dirx / dis
    local unity = diry / dis

    if (math.abs(dx - sx) > math.abs(dy - sy)) then
        troop.checkx = true
    end

    troop.wid = wid
    troop.tm = tm()
    troop.unitx = unitx
    troop.unity = unity
    troop.nowx = sx
    troop.nowy = sy
    troop.dx = dx
    troop.dy = dy
    troop.v = v

    if map[dx] and map[dx][dy] then
        local ewid = map[dx][dy]
        troop.dwid = ewid
        troop.dtype = entities[ewid].type
    end
    troops[wid] = troop
    M.add_troop_grid(troop)
end

M.reset_troop_grids = function()
    troop_grids = {}

    for wid, troop in pairs(troops) do
        M.add_troop_grid(troop)
    end
end

local del_troop_grid = function(wid, x, y)
    local gx, gy = troop_grid_pos(x, y)
    if troop_grids[gx] then
        if troop_grids[gx][gy] then
            troop_grids[gx][gy][wid] = nil
            if not next(troop_grids[gx][gy]) then
                troop_grids[gx][gy] = nil
            end
        end
        if not next(troop_grids[gx]) then
            troop_grids[gx] = nil
        end
    end
end
M.del_troop_grid = function(troop)
    if troop.dx == troop.nowx and troop.dy == troop.nowy then
        del_troop_grid(troop.wid, troop.dx, troop.dy)
        return
    end

    local tmp = {
        wid = troop.wid,
        nowx = troop.nowx,
        nowy = troop.nowy,
        dx = troop.dx,
        dy = troop.dy,
        unitx = troop.unitx,
        unity = troop.unity,
        checkx = troop.checkx
    }

    for i = 1, 2000 do
        del_troop_grid(tmp.wid, tmp.nowx, tmp.nowy)
        local arrive = M.troop_move_dis(tmp, TROOP_GRID)
        if arrive then
            del_troop_grid(tmp.wid, tmp.dx, tmp.dy)
            return
        end
    end
end

M.del_troop = function(troop)
    local wid = troop.wid
    troops[wid] = nil
    M.del_troop_grid(troop)
end

local add_troop_grid = function(wid, x, y)
    local gx, gy = troop_grid_pos(x, y)
    troop_grids[gx] = troop_grids[gx] or {}
    troop_grids[gx][gy] = troop_grids[gx][gy] or {}
    troop_grids[gx][gy][wid] = 1
end
M.add_troop_grid = function(troop)
    local tmp = {
        wid = troop.wid,
        nowx = troop.nowx,
        nowy = troop.nowy,
        dx = troop.dx,
        dy = troop.dy,
        unitx = troop.unitx,
        unity = troop.unity,
        checkx = troop.checkx
    }

    for i = 1, 2000 do
        add_troop_grid(tmp.wid, tmp.nowx, tmp.nowy)
        local arrive = M.troop_move_dis(tmp, TROOP_GRID)
        if arrive then
            add_troop_grid(tmp.wid, tmp.dx, tmp.dy)
            return
        end
    end
end

local troop_arrive_check = function(bx, nx, dx)
    if bx <= dx and dx <= nx then
        return true
    end
    if bx >= dx and dx >= nx then
        return true
    end

    if nx >= bx and bx >= dx then
        return true
    end
    if nx <= bx and bx <= dx then
        return true
    end
end

M.troop_move_dis = function(troop, dis)
    local nowx = troop.nowx + dis * troop.unitx
    local nowy = troop.nowy + dis * troop.unity
    local bp, np, dp
    if troop.checkx then
        bp = troop.nowx
        np = nowx
        dp = troop.dx
    else
        bp = troop.nowy
        np = nowy
        dp = troop.dy
    end
    if troop_arrive_check(bp, np, dp) then
        troop.nowx = troop.dx
        troop.nowy = troop.dy
        return true
    else
        troop.nowx = nowx
        troop.nowy = nowy
        -- print(difftm, troop.wid, "troop pos:", nowx, nowy)
    end
end

M.del_move_grid = function(troop, bx, by)
    local bgx, bgy = troop_grid_pos(bx, by)
    local ngx, ngy = troop_grid_pos(troop.nowx, troop.nowy)
    if bgx == ngx and bgy == ngy then
        return
    end

    local tmp = {
        wid = troop.wid,
        nowx = bx,
        nowy = by,
        dx = troop.nowx,
        dy = troop.nowy,
        unitx = troop.unitx,
        unity = troop.unity,
        checkx = troop.checkx
    }
    for i = 1, 2000 do
        del_troop_grid(tmp.wid, tmp.nowx, tmp.nowy)
        local arrive = M.troop_move_dis(tmp, TROOP_GRID)
        if arrive then
            add_troop_grid(troop.wid, troop.nowx, troop.nowy)
            return
        end
    end
end

M.troop_move = function(troop, nowtm)
    local difftm = nowtm - troop.tm
    if difftm <= 500 then
        return
    end
    troop.tm = nowtm

    local bx, by = troop.nowx, troop.nowy
    local dis = troop.v * difftm / 1000
    local arrive = M.troop_move_dis(troop, dis)
    M.del_move_grid(troop, bx, by)

    -- print("now troop pos", troop.wid, ":", troop.nowx, troop.nowy)

    if arrive then
        M.del_troop(troop)
        cb.troop_arrive(troop)
    end
end

M.troop_cross_area = function(troop, blx, bly, len)
    local nowx = troop.nowx
    local nowy = troop.nowy
    local dx = troop.dx
    local dy = troop.dy

    local trx = blx + len - 1
    local try = bly + len - 1

    if nowx >= blx and nowx <= trx and nowy >= bly and nowy <= try then
        return true
    end
    if dx >= blx and dx <= trx and dy >= bly and dy <= try then
        return true
    end

    local check_y = function(y0)
        local vy1 = y0 - nowy
        local vy2 = dy - y0
        if vy1 * vy2 <= 0 then
            return
        end
        local x0 = (vy1 * dx + vy2 * nowx) / (vy1 + vy2)
        if x0 >= blx and x0 <= trx then
            return true
        end
    end
    if check_y(bly) or check_y(try) then
        return true
    end

    local check_x = function(x0)
        local vx1 = x0 - nowx
        local vx2 = dx - x0
        if vx1 * vx2 <= 0 then
            return
        end
        local y0 = (vx1 * dy + vx2 * nowy) / (vx1 + vx2)
        if y0 >= bly and y0 <= try then
            return true
        end
    end
    if check_x(blx) or check_x(trx) then
        return true
    end
end

M.watch_troops = function(src)
    local watch_src = watches_src[src]
    local cx, cy = watch_src.cx, watch_src.cy
    local weigh = watch_src.weigh

    local weigh_times = {
        [1] = 300,
        [2] = 200,
        [3] = 100
    }

    local times = weigh_times[weigh]
    local ret = {}
    local num = 0

    local add_watch_troop = function(gx, gy)
        if num >= times then
            return
        end
        if not troop_grids[gx] then
            return
        end
        if not troop_grids[gx][gy] then
            return
        end
        for wid, _ in pairs(troop_grids[gx][gy]) do
            ret[wid] = troops[wid]
            num = num + 1
            if num >= times then
                return
            end
        end
    end

    local gx, gy = troop_grid_pos(cx, cy)
    add_watch_troop(gx, gy)
    add_watch_troop(gx + 1, gy + 1)
    add_watch_troop(gx + 1, gy - 1)
    add_watch_troop(gx - 1, gy + 1)
    add_watch_troop(gx - 1, gy - 1)
    add_watch_troop(gx + 1, gy)
    add_watch_troop(gx - 1, gy)
    add_watch_troop(gx, gy + 1)
    add_watch_troop(gx, gy - 1)

    return ret
end

M.entity_watch_area = function(entity)
    local blx = entity.blx - (WATCH_LEN - 1)
    local bly = entity.bly - (WATCH_LEN - 1)

    local len = entity.len
    return blx, bly, 2 * (WATCH_LEN - 1) + len
end

M.area_watches = function(blx, bly, len)
    blx = correct_x(blx)
    bly = correct_y(bly)
    local trx = correct_x(blx + len - 1)
    local try = correct_y(bly + len - 1)

    local arr = {}
    for i = blx, trx do
        if not watches[i] then
            goto icon
        end
        for j = bly, try do
            if not watches[i][j] then
                goto jcon
            end
            table.insert(arr, watches[i][j])
            ::jcon::
        end
        ::icon::
    end

    local MAX_NUM = 500
    local ret = {}
    for weigh = 1, 3 do
        for _, weigh_srcs in ipairs(arr) do
            local srcs = weigh_srcs[weigh]
            if not srcs then
                goto cont
            end
            for src, _ in pairs(srcs) do
                table.insert(ret, src)
                if #ret >= MAX_NUM then
                    break
                end
            end
            if #ret >= MAX_NUM then
                break
            end
            ::cont::
        end
        if #ret >= MAX_NUM then
            break
        end
    end

    return next(ret) and ret
end

M.notify_watch = function(entity, ope)
    local ws = M.area_watches(M.entity_watch_area(entity))
    if not ws then
        return
    end
    cb.notify_entity(ws, entity, ope)
end

M.watch_area = function(cx, cy)
    local blx = cx - (WATCH_LEN - 1)
    local bly = cy - (WATCH_LEN - 1)
    return blx, bly, 2 * (WATCH_LEN - 1) + 1
end

M.add_watch = function(src, cx, cy, weigh)
    weigh = weigh or 3
    M.del_watch(src)

    watches_src[src] = {
        cx = cx,
        cy = cy,
        weigh = weigh
    }

    watches[cx] = watches[cx] or {}
    watches[cx][cy] = watches[cx][cy] or {}
    watches[cx][cy][weigh] = watches[cx][cy][weigh] or {}
    watches[cx][cy][weigh][src] = 1
    local src_watch_troop = M.watch_troops(src)
    src_watch_troops[src] = src_watch_troop
    return M.area_entities(M.watch_area(cx, cy)), src_watch_troop
end

M.del_watch = function(src)
    local src_watch = watches_src[src]
    if not src_watch then
        return
    end
    watches_src[src] = nil
    src_watch_troops[src] = nil

    local cx = src_watch.cx
    local cy = src_watch.cy
    local weigh = src_watch.weigh
    if watches[cx] then
        if watches[cx][cy] then
            if watches[cx][cy][weigh] then
                watches[cx][cy][weigh][src] = nil
                if not next(watches[cx][cy][weigh]) then
                    watches[cx][cy][weigh] = nil
                end
            end
            if not next(watches[cx][cy]) then
                watches[cx][cy] = nil
            end
        end
        if not next(watches[cx]) then
            watches[cx] = nil
        end
    end
    -- print("after del watch", dump(watches), dump(watches_src))
end

local troop_tick = function(nowtm)
    if not troops then
        return
    end

    local t = skynet.now()
    for id, troop in pairs(troops) do
        M.troop_move(troop, nowtm)
    end
    -- print("troop tick cost:", #troops, skynet.now() - t)
end

local src_watch_troop_tick = function()
    local t = skynet.now()
    local ret = {}
    for src, watch_troop in pairs(src_watch_troops) do
        local new_watch_troop = M.watch_troops(src)
        src_watch_troops[src] = new_watch_troop

        for k in pairs(new_watch_troop) do
            if watch_troop[k] then
                watch_troop[k] = nil
            else
                ret[src] = ret[src] or {}
                ret[src].add = ret[src].add or {}
                local add = ret[src].add
                table.insert(add, k)
            end
        end

        for k in pairs(watch_troop) do
            ret[src] = ret[src] or {}
            ret[src].del = ret[src].del or {}
            local del = ret[src].del
            table.insert(del, k)
        end
    end
    if next(ret) then
        cb.notify_troop(ret)
    end
    -- print("src watch troop tick:", skynet.now() - t)
end

skynet.fork(function()
    while true do
        skynet.sleep(100)
        troop_tick(tm())
        src_watch_troop_tick()
        -- print(tm() % 10000)
    end
end)

M.get_map = function()
    return map
end

M.get_entities = function()
    return entities
end

M.get_troops = function()
    return troops
end

M.get_all_troopinfo = function()
    return {
        troops = troops,
        troop_grids = troop_grids
    }
end

M.get_all_watchinfo = function()
    return {
        watches = watches,
        watches_src = watches_src
    }
end

M.get_len = function()
    return world_len
end
M.get_wid = function()
    return world_wid
end

return M
