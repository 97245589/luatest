require "util"
local create_quest_mgr = require "quest"

local enums = {
    unfinish = 1,
    finish = 2,
    lv = 1,
    pet_lv_num = 2,
    acc = 3
}

local quest_cfg = {
    [10] = { target = { 1, 3 } },    -- lv >= 3
    [20] = { target = { 2, 3, 2 } }, -- 2 pets with levels >= 3
    [30] = { target = { 3, 10 } },   --accumulation test
}

local quest_mgr = create_quest_mgr({
    [enums.lv] = function(character, cfg_target, quest)
        return character.level
    end,
    [enums.pet_lv_num] = function(character, cfg_target, quest)
        local cpet = character.pet
        local i = 0
        for id, v in pairs(cpet) do
            if v.level >= cfg_target[2] then
                i = i + 1
            end
        end
        return i
    end
})

local character = {
    level = 2,
    pet = {
        [10] = { id = 10, level = 3 },
        [20] = { id = 20, level = 5 }
    },
    quest = {}
}

quest_mgr.init_quests(character, character.quest, quest_cfg)
print(dump(character.quest))

local cb = function(ch, event)
    quest_mgr.count(ch, ch.quest, quest_cfg, event)
end

quest_mgr.add_trigger_cb(1, cb)
character.level = 3
quest_mgr.trigger(character, { enums.lv, character.level })
quest_mgr.trigger(character, { enums.acc, 2 })
quest_mgr.trigger(character, { enums.acc, 3 })
print(dump(character.quest))
