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
    info = { weekly = { day = 7 }, duration = { day = 7 } }
    printt(time.parse(info))
    info = {
        { weekly = { day = 2 }, duration = { day = 1 } },
        { weekly = { day = 7 }, duration = { day = 1 } },
    }
    printt(time.parse(info))
end

local daily_test = function()
    local info
    info = { daily = { hour = 3 }, duration = { hour = 3 } }
    printt(time.parse(info))
    info = { daily = { hour = 6 }, duration = { day = 1 } }
    printt(time.parse(info))
end

local monthly_test = function()
    local info
    info = { monthly = { day = 5, hour = 10 }, duration = { month = 1 } }
    printt(time.parse(info))
    info = {
        { monthly = { day = 8 },  duration = { day = 1 } },
        { monthly = { day = 18 }, duration = { day = 1 } }
    }
    printt(time.parse(info))
end

local yearly_test = function()
    local info
    info = { yearly = { month = 12, day = 20 }, duration = { day = 20 } }
    printt(time.parse(info))
end
yearly_test()

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
