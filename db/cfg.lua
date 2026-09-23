return {
    read_cmds = {
        hget = 1,
        hmget = 1,
        hgetall = 1,
        hscan = 1,
    },
    write_cmds = {
        hset = 1,
        hmset = 1,
        hdel = 1,
        del = 1
    },
    slot_group = {
        [300] = 0,
    },
    SLOT_NUM = 1000
}
