/*
 * 相当于数据库的api，可以直接跟数据库联系
 */

#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include "muduo/net/EventLoop.h"
#include "net_generated.h"
#include "threadPool.h"
#include "dbtm/dbtm.h"

namespace grit {

// 事务的读写数据结构体
struct Data
{
    Data(std::string k, std::string lab, std::string attr, std::string val)
        : key(k)
        , label(lab)
        , attribute(attr)
        , value(val)
    {}

    std::string key;
    std::string label;
    std::string attribute;
    std::string value;
};

// 代表一个事务
struct transaction
{
    int txid;
    std::list<Data *> readSet;
    std::list<Data *> writeSet;
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
    void
    getReadWriteSet(const muduo::net::TcpConnectionPtr &, const DbServiceMsg *);

    // 用于记录txid和conn的对应
    unordered_map<int, muduo::net::TcpConnectionPtr> table;

    ThreadPool *threadPool_;
    Dbtm *dbtm_;
};

} // namespace grit