local lmap = require "lmap"
local skill = require "skill"
local enum = require "enum"

-- input = {id=,len=,wid=,data={},impl={},aoicb=func}
local create_instance = function(input)
    local len = input.len
    local wid = input.wid
    local impl = input.impl
    local aoicb = input.aoicb

    input.data = input.data or {}
    local data = input.data or {}
    data.idx = data.idx or 0
    data.entities = data.entities or {}
    local entities = data.entities

    local core = lmap.create(len, wid)
    local ins = {}
    ins.entities = entities
    ins.impl = impl
    ins.core = core

    local genid = function()
        if data.idx > 0xfffffff then
            data.idx = 0
        end
        data.idx = data.idx + 1
        return data.idx
    end

    -- obj = {id=,x=,y=,dx=,dy=,atk_type=,attr=}
    ins.add_entity = function(obj)
        obj.id = obj.id or genid()
        core:add_entity(obj.id, obj.x, obj.y, obj.dx, obj.dy, obj.atk_type)
        entities[obj.id] = obj

        local ids = core:get_aoi(obj.id)
        aoicb(ins, enum.aoi_add, obj, ids)
    end

    ins.del_entity = function(obj)
        local id = obj.id
        local ids = core:get_aoi(id)
        aoicb(ins, enum.aoi_del, obj, ids)
        entities[id] = nil
    end

    local update_pos = function(id, x, y, dx, dy, moving)
        local obj = entities[id]
        if not obj then
            return
        end
        local bx = obj.x
        local by = obj.y
        obj.x = x
        obj.y = y
        obj.dx = dx
        obj.dy = dy
        obj.moving = moving
        local add, del = core:diff_aoi(bx, by, x, y)
        if add then
            aoicb(ins, enum.aoi_add, obj, add)
            aoicb(ins, enum.aoi_del, obj, del)
        end
    end

    local last_tick_tm
    ins.tick = function(tm)
        if not last_tick_tm then
            last_tick_tm = tm
            return
        end
        local diff = (tm - last_tick_tm) * 10
        last_tick_tm = tm

        for id, entity in pairs(entities) do
            if not entity.moving then
                goto cont
            end
            if not entity.attrs then
                goto cont
            end
            ::cont::
        end
    end

    ins.use_skill = function(id, skillid)
        local entity = entities[id]
        if not entity then
            return
        end
        skill.use_skill(ins, entity, skillid)
    end

    return ins
end

return {
    create_instance = create_instance
}
