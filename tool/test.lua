local ltool = require "ltool"
local ldump = ltool.dump
local lclone = ltool.clone
require "util"

local tb = {
    id = 10,
    hello = "world",
    float = 10.010,
    arr = { "hello", 3, { [100] = { id = 100 } } },
    map = { [1000] = { id = 1000 }, [2000] = { 100, 200 } },
    -- func = function() end
}

print(ldump(tb))

local nb = lclone(tb)
print(tb, nb, ldump(nb))

local t = os.time()
for i = 1, 1000000 do
    local ttb = lclone(tb)
end
print(os.time() - t)
