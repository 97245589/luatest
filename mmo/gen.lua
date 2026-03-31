local __skillcfg = {
    [100] = { action = "damage(targ,p[1],src)" }
}
for i = 1, 5000 do
    __skillcfg[i] = {
        action = "damage(targ,p[1],src)"
    }
end

local ffunc = "static void s%s(Actor& src,Actor& targ, Param& p) {%s;}"
local finit = "func_.insert({%s, s%s});"

local funcarr = {}
local initarr = {}

for k, v in pairs(__skillcfg) do
    local func = string.format(ffunc, k, v.action)
    table.insert(funcarr, func)
    local init = string.format(finit, k, k)
    table.insert(initarr, init)
end

local finfo = [[
#include <cmath>
#include "skill.h"
using namespace Skillfunc;
using Param = Skill::Param;
%s
void Skill::init_skillfunc() {
%s
}
]]

local info = string.format(finfo, table.concat(funcarr, '\n'), table.concat(initarr, '\n'))

local f = io.open("cc/skillfunc.cc", "w")
f:write(info)
f:close()
