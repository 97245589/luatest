local ldbtool = require "ldbtool"

local VERSION
local FILE_VERSION = 1000
local DIR = "version/"
local version_path = DIR .. "version"

local curr_ins
local M = {}

local create_ins = function(minversion)
    local vf = io.open(version_path, "a")
    local core = ldbtool.create_version()
    local ins = {}
    ins.add = function(key, version)
        core:add(key, version)
        vf:write(string.pack("<i8", version))
        vf:write(string.pack("<s4", key))
        vf:flush()
    end

    ins.dump = function()
        print(minversion, VERSION)
        print(core:dump())
    end

    ins.deseri = function(bin)
        core:deseri(bin)
    end

    ins.last = function()
        return core:last()
    end

    ins.gen_file = function()
        local file_path = DIR .. minversion .. "-" .. VERSION
        local dumpf = io.open(file_path, "w")
        dumpf:write(core:seri())
        dumpf:close()
    end

    ins.del = function()
        vf:close()
        ins.gen_file()
        curr_ins = nil
    end
    curr_ins = ins
    return ins
end

M.create_ins = function(minversion)
    local vf = io.open(version_path, "w")
    vf:write(string.pack("<i8", minversion))
    vf:close()
    VERSION = minversion - 1
    return create_ins(minversion)
end

M.add = function(key)
    VERSION = VERSION + 1
    local version = VERSION
    curr_ins.add(key, version)

    if version % FILE_VERSION == 0 then
        curr_ins.del()
        M.create_ins(version + 1)
    end
end

M.revert = function()
    local vf = io.open(version_path)
    if not vf then
        return
    end
    local str = vf:read("*a")
    vf:close()
    if #str < 8 then
        return
    end
    local minversion = string.unpack("<i8", string.sub(str, 1, 8))
    local bin = string.sub(str, 9)
    local ins = create_ins(minversion)
    ins.deseri(bin)
    VERSION = ins.last() or (minversion - 1)
    return ins
end

M.init = function()
    os.execute("mkdir " .. DIR .. " >/dev/null 2>&1")
    local rok = M.revert()
    if not rok then
        M.create_ins(1)
    end
end
-- M.init()

return M
