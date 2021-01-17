#pragma once
#include "net_generated.h"
#include "dbservice/dbservice.h"
#include "muduo/net/EventLoop.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "threadPool.h"
#include "dbservice/dbservice.h"
#include "base/hash.h"
#include "base/mydb.h"
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
    void solve(const DbtmMsg *);

    // 用于判断本地是否存在冲突
    void judgeLocalConflict(struct transaction *);

    // 向dbtl获取applyLsn，参数为txid
    void getLsn(int);

    // 用于txid和对应事务的映射
    unordered_map<int, struct transaction *> table_;

  private:
    // 向gtm发送全局判冲突的请求
    void judgeGlobalConflict(int);

    // 缓存当前事务的读写集
    void cacheRWSet(struct transaction *);

    // 发送日志给LogStore
    void sendLog(struct transaction *);
    // 通过从磁盘读日志的方式发送，用在服务器宕机重启之后
    void sendLogByDisk(std::string &);

    // 将内存中的数据编程日志落盘
    // 与存储层一致，通过rocksDB进行落盘，但是有些问题，不适用当前场景
    void writeToDiskByRocksDB(struct transaction *);
    // 直接用文件系统的api进行落盘，落盘并且写入sql
    void writeToDiskAndSql(struct transaction *);

    // 连接之后的回调
    void onDbtlConnection(const TcpConnectionPtr &);
    void onGtmConnection(const TcpConnectionPtr &);

    // 初始化mysql表
    void initMySql();

    // 写死相关sql语句
    void sqlExec(std::string &);
    void sqlInsert(int, int, std::string);
    void sqlDelete(int);
    void sqlUpdateNeedLocalConflict(int, bool);
    void sqlUpdateNeedGlobalConflict(int, bool);

    // 用于快速进行数据冲突验证，用的自定义开链法hash表
    Mhash wcheck;

    // 连接dbtl和gtm
    TcpConnectionPtr dbtlConn_, gtmConn_;

    // 连接rocksDB
    // DB *rocksDb_;
    // 开启本地rocksDB
    // Options rockesDBOptions_;

    // 落盘线程池---1个线程
    ThreadPool *threadPool_;

    DbService *dbservice_;

    // 用于连接本地mysql
    MyDb db_;
};

} // namespace grit