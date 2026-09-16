#ifndef __DBIMPL_H__
#define __DBIMPL_H__

#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <vector>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"

using std::function;
using std::string;
using std::tuple;
using std::vector;

struct Dbimpl {
  struct Batch {
    Dbimpl& d_;
    leveldb::WriteBatch batch_;

    Batch(Dbimpl& d);
    ~Batch();
    void put(const string& rawkey, const string& val);
    void del(const string& key);
  };

  enum { MODE_SIMPLE, MODE_COMPLEX };
  leveldb::DB* db_ = nullptr;
  leveldb::Options options_;
  int8_t mode_ = MODE_SIMPLE;

  bool open(const string& db_name, size_t cachesize);
  ~Dbimpl();

  void compact();

  void keys(const string& patt, vector<string>& ret);
  void del(const string& key);
  void scan_key(const string&,
                function<void(const string&, const string&, const string&)> cb);
  void hgetall(const string&, vector<string>&);
  void hmget(const string&, vector<string>&, vector<bool>&);
  void hdel(const string&, const vector<string>&);
  void hmset(const string&, const vector<string>&);
  void hscan(const string&, const string&, const string&, size_t,
             vector<string>&);
};

#endif