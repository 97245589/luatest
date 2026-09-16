#include <fnmatch.h>

#include <iostream>
using namespace std;

#include "dbimpl.h"
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"

static constexpr char SPLIT = 0xff;
using Batch = Dbimpl::Batch;

Batch::Batch(Dbimpl& d) : d_(d) {}
Batch::~Batch() { d_.db_->Write(leveldb::WriteOptions(), &batch_); }
void Batch::put(const string& rawkey, const string& val) {
  batch_.Put(rawkey, val);
}
void Batch::del(const string& rawkey) { batch_.Delete(rawkey); }

bool Dbimpl::open(const std::string& db_name, int64_t cache_size) {
  options_.create_if_missing = true;
  options_.compression = leveldb::kZstdCompression;
  options_.write_buffer_size = 8 * 1024 * 1024;
  options_.max_file_size = 4 * 1024 * 1024;
  options_.block_size = 16 * 1024;

  if (cache_size > 1024 * 1024) {
    options_.block_cache = leveldb::NewLRUCache(cache_size);
    options_.filter_policy = leveldb::NewBloomFilterPolicy(10);
  }
  leveldb::Status status = leveldb::DB::Open(options_, db_name, &db_);
  if (!status.ok()) {
    return false;
  }
  return true;
}

Dbimpl::~Dbimpl() {
  if (nullptr != db_) delete db_;
  if (nullptr != options_.block_cache) delete options_.block_cache;
  if (nullptr != options_.filter_policy) delete options_.filter_policy;
}

void Dbimpl::compact() { db_->CompactRange(nullptr, nullptr); }

void Dbimpl::keys(const string& patt, vector<string>& ret) {
  ret.reserve(16);
  leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    string rawkey = it->key().ToString();
    int pos_split = rawkey.find(SPLIT);
    if (pos_split < 0 || pos_split >= rawkey.size() - 1) continue;
    string key = rawkey.substr(0, pos_split);
    if (!ret.empty() && ret.back() == key) continue;
    if (0 != fnmatch(patt.data(), key.data(), 0)) continue;
    ret.push_back(key);
  }
  delete it;
}

void Dbimpl::del(const string& key) {
  Batch batch(*this);
  scan_key(key, [&](const string& field, const string& val,
                    const string& rawkey) { batch.del(rawkey); });
}

void Dbimpl::scan_key(
    const string& key,
    function<void(const string&, const string&, const string&)> cb) {
  string start = key + SPLIT;
  string end = start + SPLIT;
  leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
  for (it->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
    string rawkey = it->key().ToString();
    string field = rawkey.substr(start.size());
    string val = it->value().ToString();
    cb(field, val, rawkey);
  }
  delete it;
}

void Dbimpl::hgetall(const string& key, vector<string>& ret) {
  ret.reserve(16);
  scan_key(key,
           [&](const string& field, const string& val, const string& rawkey) {
             ret.push_back(field);
             ret.push_back(val);
           });
}

void Dbimpl::hmget(const string& key, vector<string>& fields,
                   vector<bool>& valid) {
  valid.reserve(16);
  for (string& field : fields) {
    string rawkey = key + SPLIT + field;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), rawkey, &field);
    if (s.ok()) {
      valid.push_back(true);
    } else {
      valid.push_back(false);
    }
  }
}

void Dbimpl::hdel(const string& key, const vector<string>& fields) {
  Batch batch(*this);
  for (const string& field : fields) {
    string rawkey = key + SPLIT + field;
    batch.del(rawkey);
  }
}

void Dbimpl::hmset(const string& key, const vector<string>& field_val) {
  Batch batch(*this);
  for (size_t i = 0; i < field_val.size() - 1; i += 2) {
    const string& rawkey = key + SPLIT + field_val[i];
    const string& val = field_val[i + 1];
    batch.put(rawkey, val);
  }
}

void Dbimpl::hscan(const string& key, const string& cursor, const string& patt,
                   size_t count, vector<string>& ret) {
  ret.reserve(21);
  string start = key + SPLIT + cursor;
  string end = key + SPLIT + SPLIT;
  leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
  ret.push_back("0");
  for (it->Seek(start); it->Valid() && it->key().ToString() < end; it->Next()) {
    string rawkey = it->key().ToString();
    string field = rawkey.substr(key.size() + 1);
    string val = it->value().ToString();
    if (0 != fnmatch(patt.data(), field.data(), 0)) continue;
    if (ret.size() - 1 < 2 * count) {
      ret.push_back(field);
      ret.push_back(val);
    } else {
      ret[0] = field;
      break;
    }
  }
  delete it;
}