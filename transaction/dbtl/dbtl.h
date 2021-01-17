/*
 *   以固定格式整理日志，并通过远程调用数据库api的方式来将数据写入数据库
 */

#pragma once
#include "net_generated.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "muduo/net/TcpConnection.h"
#include "base/threadPool.h"
#include "logplayer.h"
#include <unordered_map>
#include <list>
namespace grit {

struct LogInfo
{
    LogInfo(std::string k, std::string attr, std::string val)
        : key(k)
        , attribute(attr)
        , value(val)
    {}
    std::string key;
    std::string attribute;
    std::string value;
};
class Dbtl
{
    friend class LogPlayer;

  public:
    Dbtl(EventLoop *);

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
    // TODO: applyLsn还没更新
    atomic<int> lsn_, applyLsn_;

    // txid-->LogInfo*
    unordered_map<int, list<struct LogInfo *> > logTable_;

    // 用来进行日志的分发工作
    ThreadPool *pool_;

    // 日志分发器
    LogPlayer *logPlayer_;
};

} // namespace grit