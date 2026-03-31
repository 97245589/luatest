require "util"

local skill = {
    HP = 1,
    ATK = 10,
    DEF = 20
}
skill.fattr = function(unit, k)
    local attrs = unit.attrs
    local base = attrs[k] or 0
    local add = attrs[k + 1] or 0
    local per = attrs[k + 2] or 0
    return (base + add) * (1 + per)
end
skill.damage = function(targ, val, src)
    -- print(val)
end

local __skillcfg = {
    [100] = {
        action = "damage(targ, fattr(src,20)-fattr(targ,10))"
    }
}

local faction = "return function(src,targ) %s end"
for k, v in pairs(__skillcfg) do
    local str = string.format(faction, v.action)
    v.action = load(str, "skill" .. k, "bt", skill)()
end

local src = {
    attrs = {
        [skill.ATK] = 20,
        [skill.DEF] = 10,
    }
}
local targ = clone(src)

local action = __skillcfg[100].action
action(src, targ)

local t = os.time()
for i = 1, 1000000 do
    action(src, targ)
end
print(os.time() - t)
