local pairs       = pairs
local ipairs      = ipairs
local table       = table
local string      = string
local M           = {}

local meta
M.setmeta         = function(m)
    meta = m
end

local parse_dep   = function(name)
    local meta_dep = {}
    local dep_info = {}
    local parse
    parse = function(name, depth)
        local minfo = meta[name]
        for _, ele in ipairs(minfo) do
            local tp = ele[2]
            local val = ele[3]
            if tp == "obj" or tp == "arr" or tp == "map" then
                parse(val, depth + 1)
            end
        end

        local mdep = meta_dep[name]
        if mdep and mdep >= depth then
            return
        end

        if mdep then
            meta_dep[name] = nil
            dep_info[mdep][name] = nil
        end
        dep_info[depth] = dep_info[depth] or {}
        dep_info[depth][name] = 1
        meta_dep[name] = depth
    end
    parse(name, 1)
    return dep_info
end

local parse_db_cb = function(name, func)
    local dep_info = parse_dep(name)
    for i = #dep_info, 1, -1 do
        local info = dep_info[i]
        for metaname in pairs(info) do
            func(metaname)
        end
    end
end

M.genobj          = function(name)
    local obj = {}
    local one_obj = function(name)
        local ret = {}
        local minfo = meta[name]
        for _, ele in ipairs(minfo) do
            local k = ele[1]
            local tp = ele[2]
            local val = ele[3]
            if tp == "obj" then
                ret[k] = obj[val]
            elseif tp == "arr" or tp == "map" then
                -- ret[k] = { ["__" .. tp] = val }
                ret[k ] = {}
            else
                ret[k] = val
            end
        end
        obj[name] = ret
    end
    parse_db_cb(name, function(metaname)
        one_obj(metaname)
    end)
    return obj
end

M.sproto          = function(name)
    local type_handle = {
        integer = function()
            return "integer"
        end,
        double = function()
            return "double"
        end,
        string = function()
            return "string"
        end,
        obj = function(val)
            return val
        end,
        arr = function(val)
            return "*" .. val
        end,
        map = function(val)
            return "*" .. val .. "(id)"
        end
    }
    local one_db_proto = function(name)
        local fmt = ".%s {\n%s\n}"
        local arr = {}
        local minfo = meta[name]
        for idx, ele in ipairs(minfo) do
            local k = ele[1]
            local tp = ele[2]
            local val = ele[3]
            local stp = type_handle[tp](val)
            local elestr = string.format("%s %s : %s", k, idx, stp)
            table.insert(arr, elestr)
        end
        return string.format(fmt, name, table.concat(arr, '\n'))
    end

    local arr = {}
    parse_db_cb(name, function(metaname)
        table.insert(arr, one_db_proto(metaname))
    end)
    return table.concat(arr, '\n')
end

return M
