require "util"

local time = require "time"
local printst = function(s, e)
    if not s then
        print(s, e)
        return
    end
    print(time.format(s), time.format(e))
end


local testeveryweek = function()
    local cfg = {
        everyweek = { weekday = 1, hour = 5 }, duration = { day = 3 },
    }
    printst(time.startendtm(cfg))

    local cfg = {
        everyweek = { weekday = 1 }, duration = { day = 1 }
    }
    printst(time.startendtm(cfg))

    local cfg = {
        { everyweek = { weekday = 1 }, duration = { day = 1 } },
        { everyweek = { weekday = 7 }, duration = { day = 1 } }
    }
    printst(time.startendtm(cfg))
end
testeveryweek()

local testweek = function()
    time.set_startts(time.day_start())
    local cfg = {
        week = { startweek = 1, weekday = 6 }, duration = { day = 3 },
    }
    printst(time.startendtm(cfg))

    local cfg = {
        { week = { startweek = 1, weekday = 1 }, duration = { day = 3 } },
        { week = { startweek = 1, weekday = 5 }, duration = { day = 3 } }
    }
    printst(time.startendtm(cfg))
end

local testafter = function()
    time.set_startts(time.day_start())
    local cfg = {
        afterstart = { day = 3, hour = 10 }, duration = { day = 3 }
    }
    printst(time.startendtm(cfg))

    time.set_startts(os.time({ year = 2026, month = 2, day = 15, hour = 0 }))
    local cfg = {
        { afterstart = {},          duration = { day = 2 } },
        { afterstart = { day = 3 }, duration = { day = 2 } },
    }
    printst(time.startendtm(cfg))
end

local testtime = function()
    local cfg = {
        time = { year = 2026, month = 2, day = 10 },
        duration = { day = 10 },
    }
    printst(time.startendtm(cfg))
    local cfg = {
        time = { year = 2026, month = 2, day = 15, hour = 10 },
        duration = { day = 3 }
    }
    printst(time.startendtm(cfg))
    local cfg = {
        { time = { year = 2026, month = 2, day = 16 }, duration = { day = 2 } },
        { time = { year = 2026, month = 2, day = 20 }, duration = { day = 2 } },
    }
    printst(time.startendtm(cfg))
end
