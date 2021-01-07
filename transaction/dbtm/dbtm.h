#pragma once
#include "net_generated.h"
#include "dbservice/dbservice.h"
#include "muduo/net/EventLoop.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "threadPool.h"
#include "dbservice/dbservice.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace grit {

class Dbtm
{
  public:
    Dbtm(DbService *);
    ~Dbtm();

    // 初始化，用于连接LogStore服务器和GTM服务器
    void init(EventLoop *);

    // 用于处理lsn和gtm冲突处理的结果
    void solve(const flat::DbService *);

    // 用于判断本地是否存在冲突
    void judgeLocalConflict(struct transaction *);

  private:
    // 获取lsn并且判断事务是否存在全局冲突
    void getLsnAndGlobalConflict(int);

    // 缓存当前事务的读写集
    void cacheRWSet(struct transaction *);

    // 发送日志给LogStore
    void sendLog(struct transaction *);
    // 通过从磁盘读日志的方式发送，用在服务器宕机重启之后
    void sendLogByDisk(std::string &);

    // 将内存中的数据编程日志落盘
    // 与存储层一直，通过rocksDB进行落盘，但是有些问题，不适用当前场景
    void writeToDiskByRocksDB(struct transaction *);
    void writeToDisk(struct transaction *);

    // 连接之后的回调
    void onDbtlConnection(const muduo::net::TcpConnectionPtr &);
    void onGtmConnection(const muduo::net::TcpConnectionPtr &);

    // 用于快速进行数据冲突验证
    // TODO: 数据验证好像可以用位图来做，参考底层的分布式图数据库存储
    // 第一个是lsn
    std::unordered_map<std::string, std::unordered_set<std::string> > rcheck,
        wcheck;

    // 用于txid和对应事务的映射
    std::unordered_map<int, struct transaction *> table;

    // 连接dbtl和gtm
    TcpConnectionPtr dbtlConn_, gtmConn_;

    // 连接rocksDB
    DB *rocksDb_;
    // 开启本地rocksDB
    Options rockesDBOptions_;

    // 落盘线程池-1个线程
    ThreadPool *threadPool_;

    DbService *dbservice_;
};

} // namespace grit