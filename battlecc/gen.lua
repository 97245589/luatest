local __skillcfg = {
    [100] = {
        action = "float v=fattr(src_,ATK)*p_[1]-fattr(targ_,DEF);damage(v);",
    }
}
-- for i = 1, 1000 do
--     __skillcfg[i] = {
--         action = "float v=fattr(src_,ATK)*p_[1]-fattr(targ_,DEF);damage(v);"
--     }
-- end

local funcarr = {}
local fstr = "skillfunc_[%s]=[=](){%s};"

for k, cfg in pairs(__skillcfg) do
    local str = string.format(fstr, k, cfg.action)
    table.insert(funcarr, str)
end

local finfo = [[
#include "battle.h"
void Skill::initfunc() {
%s
}
]]

local info = string.format(finfo, table.concat(funcarr, '\n'))

local f = io.open("cc/skillfunc.cc", "w")
f:write(info)
f:close()
