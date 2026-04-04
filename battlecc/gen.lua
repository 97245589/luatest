local __skillcfg = {
    [101] = {
        action =
        "float v = fattr(src,ATK)*p[1]; buff_roundend(targ,src,1,p,[=](Actor& actor, Buff& buff){addhp(actor, v);});"
    }
}
-- for i = 1, 5000 do
--     __skillcfg[i] = {
--     action =
--         "float v = fattr(src,ATK)*p[1]; buff_roundend(targ,src,1,p,[=](Actor& actor, Buff& buff){addhp(actor, v);});"
--     }
-- end

local farr = {}
local fstr = "static void s%s(Actor& src, Actor& targ, Param p) {%s}"
local iarr = {}
local istr = "skillfunc_.insert({%s, s%s});"
for k, cfg in pairs(__skillcfg) do
    local f = string.format(fstr, k, cfg.action)
    table.insert(farr, f)
    local i = string.format(istr, k, k)
    table.insert(iarr, i)
end

local finfo = [[
#include "battle.h"
using namespace Skillfunc;
%s
void Skill::initfunc() {
%s
}
]]

local info = string.format(finfo, table.concat(farr, '\n'), table.concat(iarr, '\n'))

local f = io.open("cc/skillfunc.cc", "w")
f:write(info)
f:close()
