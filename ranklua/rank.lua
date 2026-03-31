local table = table

local less = function(lhs, rhs)
    if lhs.score ~= rhs.score then
        return lhs.score > rhs.score
    end
    if lhs.tm ~= rhs.tm then
        return lhs.tm < rhs.tm
    end
    return lhs.id < rhs.id
end

local lower_bound = function(rank, data)
    local rankarr = rank.arr
    local left, right = 1, #rankarr
    local ans = #rankarr + 1
    while left <= right do
        local mid = (left + right) // 2
        if less(rankarr[mid], data) then
            left = mid + 1
        else
            ans = mid
            right = mid - 1
        end
    end
    return ans
end

local binary_search = function(rank, data)
    local rankarr = rank.arr
    local left, right = 1, #rankarr
    while left <= right do
        local mid = (left + right) // 2
        local rdata = rankarr[mid]
        if less(data, rdata) then
            right = mid - 1
        elseif less(rdata, data) then
            left = mid + 1
        else
            return mid, rdata
        end
    end
end

local M = {}

M.new = function(num)
    return {
        num = num,
        arr = {},
        map = {}
    }
end

M.add = function(rank, data)
    local rankmap, rankarr, maxnum = rank.map, rank.arr, rank.num
    local bdata = rankmap[data.id]
    if bdata then
        local pos = binary_search(rank, bdata)
        table.remove(rankarr, pos)
        rankmap[data.id] = nil
    end
    rankmap[data.id] = data
    local pos = lower_bound(rank, data)
    table.insert(rankarr, pos, data)
    if #rankarr > maxnum then
        local last = rankarr[#rankarr]
        table.remove(rankarr, #rankarr)
        rankmap[last.id] = nil
    end
end

M.info = function(rank, lb, ub)
    local arr = rank.arr
    if ub > #arr then
        ub = #arr
    end
    local ret = {}
    for i = lb, ub do
        table.insert(ret, arr[i])
    end
    return ret
end

M.get_order = function(rank, id)
    local rankmap = rank.map
    local data = rankmap[id]
    if not data then
        return
    end
    return binary_search(rank, data)
end

return M
