require "util"
local parsemeta = require "parsemeta"
local playermeta = require "playermeta"

parsemeta.setmeta(playermeta)

print(parsemeta.sproto("Player"))
local obj = parsemeta.genobj("Player")
print(dump(obj.Player))
