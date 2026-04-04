local require = require
require "util"
local clone = clone
local skill = require "skill"
local battle = require "battle"

local unit = {
    skills = {},
    attrs = {
        [skill.HP] = 100,
        [skill.ATK] = 20,
        [skill.DEF] = 10,
    },
}
local atk = {
    [1] = clone(unit),
    [2] = clone(unit),
    [5] = clone(unit)
}
local def = {
    [2] = clone(unit),
    [5] = clone(unit),
    [6] = clone(unit)
}

battle.start(atk, def)

local t = os.time()
for i = 1, 10000 do
    battle.start(atk, def)
end
print(os.time() - t)
