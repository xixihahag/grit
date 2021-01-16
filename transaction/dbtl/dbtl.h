/*
 *   以固定格式整理日志，并通过远程调用数据库api的方式来将数据写入数据库
 */

#pragma once
#include "net_generated.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "muduo/net/TcpConnection.h"
namespace grit {
class Dbtl
{
  public:
    Dbtl();

    void solve(const muduo::net::TcpConnectionPtr &, const flat::DbtlMsg *);

  private:
    void writeToDisk(
        int txid,
        const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *,
        const muduo::net::TcpConnectionPtr &);
    void writeToDiskByRocksDb(
        int txid,
        const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *,
        const muduo::net::TcpConnectionPtr &);

    // 给上层返回结果
    void retResult(int, int, int, const muduo::net::TcpConnectionPtr &);

    // 连接rocksDB
    DB *rocksDb_;
    // 开启本地rocksDB
    Options rockesDBOptions_;

    // 第一个是分发出去的lsn，第二个是已经落盘的lsn号
    atomic<int> lsn_, applyLsn_;
};

} // namespace grit