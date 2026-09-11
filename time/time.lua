local M = {}

M.printt = function(...)
    local arr = table.pack(...)
    for _, ts in ipairs(arr) do
        print(os.date("%Y-%m-%d %H:%M:%S", ts))
    end
    print("======")
end

local day_start = function(ts)
    local tb = os.date("*t", ts)
    tb.hour = 0
    tb.min = 0
    tb.sec = 0
    tb.isdst = nil
    return os.time(tb)
end

local week_start = function(ts)
    local tb = os.date("*t", ts)
    tb.hour = 0
    tb.min = 0
    tb.sec = 0
    local wday = tb.wday - 1
    if 0 == wday then
        wday = 7
    end
    tb.day = tb.day - (wday - 1)
    tb.isdst = nil
    return os.time(tb), tb
end

local server_open_time = day_start()
M.set_open_time = function(tm)
    server_open_time = tm
end

local add_duration = function(ts, dura_tb)
    local time_tb = os.date("*t", ts)
    for k, v in pairs(dura_tb) do
        time_tb[k] = time_tb[k] + v
    end
    time_tb.isdst = nil
    return os.time(time_tb), time_tb
end

local parse_date = function(info)
    local date_tb = info.date
    date_tb.hour = date_tb.hour or 0
    date_tb.min = date_tb.min or 0
    date_tb.sec = date_tb.sec or 0
    local start_ts = os.time(date_tb)
    local end_ts = add_duration(start_ts, info.duration)
    if os.time() < end_ts then
        return start_ts, end_ts
    end
end

local parse_weekly = function(info)
    local _, time_tb = add_duration(week_start(), info.weekly)

    local nowts = os.time()
    time_tb.day = time_tb.day - 1 - 7 * 2
    for i = 1, 3 do
        time_tb.day = time_tb.day + 7
        local start_ts = os.time(time_tb)
        local end_ts = add_duration(start_ts, info.duration)
        if nowts < end_ts then
            return start_ts, end_ts
        end
    end
end

local parse_daily = function(info)
    local time_tb = os.date("*t")
    time_tb.isdst = nil
    local info_daily = info.daily
    time_tb.hour = info_daily.hour or 0
    time_tb.min = info_daily.min or 0
    time_tb.sec = info_daily.sec or 0

    local nowts = os.time()
    time_tb.day = time_tb.day - 2
    for i = 1, 3 do
        time_tb.day = time_tb.day + 1
        local start_ts = os.time(time_tb)
        local end_ts = add_duration(start_ts, info.duration)
        if nowts < end_ts then
            return start_ts, end_ts
        end
    end
end

local parse_monthly = function(info)
    local time_tb = os.date("*t")
    time_tb.isdst = nil
    local info_monthly = info.monthly
    time_tb.day = info_monthly.day or 1
    time_tb.hour = info_monthly.hour or 0
    time_tb.min = info_monthly.min or 0
    time_tb.sec = info_monthly.sec or 0

    local nowts = os.time()
    time_tb.month = time_tb.month - 2
    for i = 1, 3 do
        time_tb.month = time_tb.month + 1
        local start_ts = os.time(time_tb)
        local end_ts = add_duration(start_ts, info.duration)
        if nowts < end_ts then
            return start_ts, end_ts
        end
    end
end

local parse_yearly = function(info)
    local time_tb = os.date("*t")
    time_tb.isdst = nil
    local info_yearly = info.yearly
    time_tb.month = info_yearly.month or 1
    time_tb.day = info_yearly.day or 1
    time_tb.hour = info_yearly.hour or 0
    time_tb.min = info_yearly.min or 0
    time_tb.sec = info_yearly.sec or 0

    local nowts = os.time()
    time_tb.year = time_tb.year - 2
    for i = 1, 3 do
        time_tb.year = time_tb.year + 1
        local start_ts = os.time(time_tb)
        local end_ts = add_duration(start_ts, info.duration)
        if nowts < end_ts then
            return start_ts, end_ts
        end
    end
end

local parse_afteropen_period = function(info)
    local start_ts = add_duration(server_open_time, info.afteropen_period)
    local end_ts = add_duration(start_ts, info.duration)
    if os.time() < end_ts then
        return start_ts, end_ts
    end
end

local parse_afteropen_week = function(info)
    local _, time_tb = add_duration(week_start(server_open_time), info.afteropen_week)
    time_tb.day = time_tb.day - 1
    local wts = os.time(time_tb)
    if wts < server_open_time then
        time_tb.day = time_tb.day + 7
        wts = os.time(time_tb)
    end
    local nth_week = info.nth_week or 1
    local start_ts = add_duration(wts, { day = 7 * (nth_week - 1) })
    local end_ts = add_duration(start_ts, info.duration)
    if os.time() < end_ts then
        return start_ts, end_ts
    end
end

local switch = {
    date = parse_date,
    daily = parse_daily,
    monthly = parse_monthly,
    yearly = parse_yearly,
    weekly = parse_weekly,
    afteropen_period = parse_afteropen_period,
    afteropen_week = parse_afteropen_week
}

local parse = function(info)
    for k, f in pairs(switch) do
        if info[k] then
            return f(info)
        end
    end
end

M.parse = function(info)
    if #info == 0 then
        info = { info }
    end

    local retarr = {}
    for _, inf in ipairs(info) do
        local s, e = parse(inf)
        if s then
            table.insert(retarr, s)
            table.insert(retarr, e)
        end
    end

    local min_ts = math.maxinteger
    local start_ts, end_ts
    for i = 1, #retarr, 2 do
        local p1 = retarr[i]
        local p2 = retarr[i + 1]
        if p1 < min_ts then
            min_ts = p1
            start_ts = p1
            end_ts = p2
        end
    end
    return start_ts, end_ts
end

return M
