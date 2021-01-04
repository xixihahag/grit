/*
 * 相当于数据库的api，可以直接跟数据库联系
 */

#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include "muduo/net/EventLoop.h"
#include "net_generated.h"
#include "threadPool.h"
#include "dbtm/dbtm.h"

namespace grit {

// 事务的读集合
struct writeData
{
    writeData(std::string k, std::string rec, std::string val)
        : key(k)
        , record(rec)
        , value(val)
    {}

    std::string key;
    std::string record;
    std::string value;
};

// 事务的写集合
struct readData
{
    readData(std::string k)
        : key(k)
    {}

    std::string key;
};

// 代表一个事务
struct transaction
{
    int txid;
    std::list<readData *> readSet;
    std::list<writeData *> writeSet;
    bool isConflict;
    std::string lsn;
    std::unordered_set<std::string> trcheck, twcheck;
};

class DbService
{
  private:
    // 用于数据库的连接,暂时无用
    std::string ip_;
    int port_;
    std::string database_;
    std::string passwd_;

  public:
    DbService(muduo::net::EventLoop *);

    DbService(
        std::string ip,
        int port,
        std::string database,
        std::string passwd)
        : ip_(ip)
        , port_(port)
        , database_(database)
        , passwd_(passwd)
    {}

    void test();

    // 连接远端的数据库
    void connectToDatabase();

    // 从传输过来的数据中解析读写集
    void getReadWriteSet(const flat::DbService *);

    ThreadPool *threadPool_;
    Dbtm *dbtm_;
};

} // namespace grit