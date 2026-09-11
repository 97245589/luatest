require "util"
local time = require "time"
local printt = time.printt

time.set_open_time(os.time(
    { year = 2026, month = 9, day = 8, hour = 0, min = 0, sec = 0 }
))

local afteropen_week = function()
    local info
    info = {
        afteropen_week = { day = 1, hour = 10 }, nth_week = 2, duration = { day = 1 }
    }
    printt(time.parse(info))
    info = {
        { afteropen_week = { day = 1 }, duration = { day = 1 } },
        { afteropen_week = { day = 5 }, duration = { day = 1 } }
    }
    printt(time.parse(info))
end

local afteropen_period = function()
    local info
    info = {
        afteropen_period = { day = 3 }, duration = { day = 1 }
    }
    printt(time.parse(info))
    info = {
        { afteropen_period = {},          duration = { day = 1 } },
        { afteropen_period = { day = 2 }, duration = { day = 1 } }
    }
    printt(time.parse(info))
end

local weekly_test = function()
    local info
    info = { weekly = { day = 3, hour = 10 }, duration = { day = 1 } }
    printt(time.parse(info))
    info = {
        { weekly = { day = 2 }, duration = { day = 1 } },
        { weekly = { day = 7 }, duration = { day = 1 } },
    }
    printt(time.parse(info))
end

local date_test = function()
    local info
    info = {
        date = { year = 2026, month = 9, day = 8 }, duration = { day = 2 }
    }
    printt(time.parse(info))
    info = {
        { date = { year = 2026, month = 9, day = 8 },  duration = { day = 2 } },
        { date = { year = 2026, month = 9, day = 15 }, duration = { day = 1 } }
    }
    printt(time.parse(info))
end
