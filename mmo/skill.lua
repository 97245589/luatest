local enum = require "enum"

local __skill_cfg = {
    [100] = { action = "damage(caster,target,p[1])", targ = { 30, 0, 1, 10 }, p = { 10 } },
    [101] = { p = { 20 } },
    [200] = {
        action = "damage(caster, target, p[1]*fattr(caster, atk)-fattr(target, def))",
        targ = { 30, 0, 1, 10 },
        p = { 1 }
    },
    [201] = { p = { 2 } },
    [300] = { action = "buff_attr(target, 1, p)", targ = { 1, 2 }, p = { 32, 0.5 } }
}

local M = {
    caster = nil,
    target = nil,
    p = nil,
    map = nil,
    hp = enum.attr_hp,
    atk = enum.attr_atk,
    def = enum.attr_def,
}

local init_skill = function()
    local idarr = {}
    for id in pairs(__skill_cfg) do
        table.insert(idarr, id)
    end
    table.sort(idarr)

    local sid, action, targ, p
    for _, id in ipairs(idarr) do
        local tid = id // 100
        if tid ~= sid then
            sid = tid
            action = nil
            targ = nil
            p = nil
        end
        local cfg = __skill_cfg[id]
        if cfg.action then
            cfg.action = load(cfg.action, "skill" .. id, "bt", M)
            action = cfg.action
        else
            cfg.action = action
        end
        if cfg.targ then
            targ = cfg.targ
        else
            cfg.targ = targ
        end
        if cfg.p then
            p = cfg.p
        else
            cfg.p = p
        end
    end
end
init_skill()

M.damage = function(caster, target, val)
    local impl = M.map.impl
    local damagef = impl.damage
    local dief = impl.die

    if damagef then
        damagef(caster, target, val)
    end

    local attr = target.attr
    local rhp = attr[enum.attr_rhp] or 0
    attr[enum.attr_rhp] = rhp - val

    if attr[enum.attr_rhp] < 0 and dief then
        dief(caster, target)
    end
end

M.add_hp = function(caster, target, num)
    local attr = target.attr
    local v = attr[enum.attr_rhp] or 0
    attr[enum.attr_rhp] = v + num
end

M.buff_attr = function(caster, target, buffid, attr)
    local gen_id = function()
        local buff = target.buff
        if buff.idx > 0xffffff then
            buff.idx = 0
        end
        buff.idx = buff.idx + 1
        return buff.idx
    end
    local data = target.buff.data
    local id = gen_id()
    data[id] = {
        id = id,
        buffid = buffid,
        attr = attr,
        end_tm = nil
    }
    local eattr = target.attr
    for k, v in attr do
        local ov = eattr[k] or 0
        eattr[k] = ov + v
        ov = ov + v
    end
end

M.del_buff = function(entity, id)
    local data = entity.buff.data
    local buff = data[id]
    data[id] = nil
    if not buff then
        return
    end
    local eattr = entity.attr
    if buff.attr then
        for k, v in pairs(buff.attr) do
            local ov = eattr[k] or 0
            eattr[k] = ov - v
        end
    end
end

M.fattr = function(entity, k)
    local eattr = entity.attr
    local base = eattr[k] or 0
    local add = eattr[k + 1] or 0
    local per = eattr[k + 2] or 0
    return (base + add) * (1 + per)
end

M.use_skill = function(map, caster, skillid)
    local cfg = __skill_cfg[skillid]
    if not cfg then
        return
    end
    local num, atk_tp, range_tp, p1, p2 = table.unpack(cfg.targ)
    local ids
    if atk_tp == 2 then
        ids = { caster.id }
    else
        ids = map.core:aoi_search(caster.id, num, atk_tp, range_tp, p1, p2)
    end
    if not ids then
        return
    end
    M.map = map
    local action = cfg.action
    M.caster = caster
    M.p = cfg.p
    local entities = map.entities
    for _, oid in ipairs(ids) do
        local entity = entities[oid]
        if not entity then
            goto cont
        end
        M.target = entity
        action()
        ::cont::
    end
end

return M
