local meta = {}

--[[
[1] fileld k
[2] type integer double string obj arr map
[3] val defaultval(integer double string) type(obj,arr,map)
[4] nil(db and client) 1(db no client) 2(client no db)
]]
meta.Player = {
    { "role",    "obj", "Role" },
    { "items",   "arr", "Item" },
    { "itemmap", "map", "Item" }
}

meta.Role   = {
    { "playerid", "integer", 0 },
    { "level",    "double",  1 },
    { "name",     "string",  "" },
}

meta.Item   = {
    { "id",  "integer", 0 },
    { "num", "integer", 0 }
}

meta.attr = {}

return meta
