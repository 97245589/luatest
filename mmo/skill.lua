local enum = require "enum"

local __skill_cfg = {
    [100] = { action = "damage(caster,target,p[1])", targ = { 30, 0, 1, 10 }, param = { 10 } },
    [101] = { param = { 20 } }
}

local M = {
    caster = nil,
    target = nil,
    p = nil,
    map = nil
}

local init_skill = function()
    local sid, action, targ, param
    for id, scfg in pairs(__skill_cfg) do
        local tid = id // 100
        if tid ~= sid then
            sid = tid
            action = nil
            targ = nil
            param = nil
        end
        if scfg.action then
            scfg.action = load(scfg.action, "skill" .. id, "bt", M)
            action = scfg.action
        else
            scfg.action = action
        end
        if scfg.targ then
            targ = scfg.targ
        else
            scfg.targ = targ
        end
        if scfg.param then
            param = scfg.param
        else
            scfg.param = param
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
    local hp = attr[enum.attr_hp]
    hp = hp - val
    if hp < 0 and dief then
        dief(caster, target)
    end
    attr[enum.attr_hp] = hp
end

M.fattr = function(entity, k)
    k = k // 100 * 100
    return entity.attr[k + 5] or 0
end

M.use_skill = function(map, caster, skillid)
    local cfg = __skill_cfg[skillid]
    if not cfg then
        return
    end
    local ids = map.aoi_search(caster.id, table.unpack(cfg.targ))
    if not ids then
        return
    end
    M.map = map
    local action = cfg.action
    M.caster = caster
    M.p = cfg.param
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

local nocal = {
    [enum.attr_hp] = 1,
    [enum.attr_mp] = 1
}
M.cal_attr = function(entity)
    local attr = entity.attr
    if not attr then
        return
    end
    local cal = {}
    for k in pairs(attr) do
        if nocal[k] then
            goto cont
        end
        local ck = k // 100 * 100
        cal[ck] = 1
        ::cont::
    end
    for k in pairs(cal) do
        local base = attr[k] or 0
        local add = attr[k + 1] or 0
        local per = attr[k + 2] or 0
        attr[k + 5] = (base + add) * (1 + per)
    end
end

return M
