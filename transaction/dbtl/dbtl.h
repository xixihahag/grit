/*
 *   以固定格式整理日志，并通过远程调用数据库api的方式来将数据写入数据库
 */

#pragma once
#include "net_generated.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"

namespace grit {

class Dbtl
{
  public:
    Dbtl();

    void solve(const flat::Dbtl *);

  private:
    void writeToDisk(
        int txid,
        const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *);
    void writeToDiskByRocksDb(
        int txid,
        const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *);

    // 连接rocksDB
    DB *rocksDb_;
    // 开启本地rocksDB
    Options rockesDBOptions_;

    // TODO: lsn应该是这边负责维护的
};

} // namespace grit