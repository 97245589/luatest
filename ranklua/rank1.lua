require "util"

local create_rank = function(maxnum)
    local rankarr = {}
    local rankmap = {}
    local M = {
        rankarr = rankarr,
        rankmap = rankmap
    }

    local less = function(lhs, rhs)
        if lhs.score ~= rhs.score then
            return lhs.score > rhs.score
        end
        if lhs.tm ~= rhs.tm then
            return lhs.tm < rhs.tm
        end
        return lhs.id < rhs.id
    end

    local lower_bound = function(data)
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

    local binary_search = function(data)
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

    M.binary_search = binary_search
    M.lower_bound = lower_bound

    --[[
    data = {score=, id=, tm=}
    ]]
    M.add = function(data)
        local bdata = rankmap[data.id]
        if bdata then
            local pos = M.binary_search(bdata)
            table.remove(rankarr, pos)
            rankmap[data.id] = nil
        end
        rankmap[data.id] = data
        local pos = lower_bound(data)
        table.insert(rankarr, pos, data)
        if #rankarr > maxnum then
            local last = rankarr[#rankarr]
            table.remove(rankarr, #rankarr)
            rankmap[last.id] = nil
        end
    end

    M.get_order = function(id)
        local data = rankmap[id]
        if not data then
            return
        end
        return binary_search(data)
    end

    return M
end

local test = function()
    local rank = create_rank(5)
    for i = 1, 20 do
        local id = math.random(10)
        local score = math.random(20)
        print(string.format("id: %s, score: %s", id, score))
        rank.add({
            id = id,
            score = score,
            tm = i
        })
    end
    print(dump(rank.rankarr), dump(rank.rankmap))

end

local stress = function()
    local rank = create_rank(1000)
    local t = os.time()
    for i = 1, 100000 do
        rank.add({
            id = i % 2000,
            score = i,
            tm = i
        })
    end
    print(os.time() - t, #rank.rankarr)

    local t = os.time()
    local ret = {}
    for i = 1, 50000 do
        for i = 1, #rank.rankarr do
            ret[i] = rank.rankarr[i]
        end
    end
    print(os.time() - t, #ret)

    local t = os.time()
    for i = 1, 500000 do
        rank.get_order(1010)
    end
    print(os.time() - t, rank.get_order(1010))
end
stress()
