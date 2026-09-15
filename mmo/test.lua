require "util"
local enum = require "enum"
local map = require "map"

local ins = map.create_instance({
    id = 100,
    len = 1024,
    wid = 1024,
    impl = {
        damage = function(caster, target, val)
            print(caster.id, target.id, val)
        end,
        die = function(caster, target)
        end
    },
    aoicb = function(ins, opt, obj, ids)
        -- print(opt, obj.id, dump(ids))
    end
})

for i = 1, 10 do
    ins.add_entity({
        id = i,
        x = i,
        y = 0,
        dx = 1,
        dy = 1,
        atk_type = i % 2,
        attr = {
            [enum.attr_hp] = 100,
            [enum.attr_rhp] = 100,
            [enum.attr_atk] = 20,
            [enum.attr_def] = 10
        },
        buff = { idx = 0, data = {}, stack = {} }
    })
end

ins.use_skill(5, 300)
print(dump(ins.entities[5].attr))
