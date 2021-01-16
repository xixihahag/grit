/*
 * 相当于数据库的api，可以直接跟数据库联系
 */

#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"
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
    transaction()
        : needGlobalConflct(false)
        , lsn(-1) // 三种状态，-1代表还未发送获取lsn请求，0代表已经发送请求但是还未收到结果，>0表示收到lsn结果
        , alreadyCommit(false)
    {}

    int txid;
    std::list<Data *> readSet;
    std::list<Data *> writeSet;
    bool needGlobalConflct;
    int lsn;

    // 用于处理得到lsn前就已经commit的情况
    bool alreadyCommit;
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

    // 连接远端的数据库
    void connectToDatabase();

    // 从传输过来的数据中解析读写集
    void getReadWriteSet(const TcpConnectionPtr &, const DbServiceMsg *);

    // 用于向上层返回事务结果，第一个参数为事务执行结果，第二个参数为txid
    void retResult(int, int);

    // 记录txid和es的conn的对应，用于向上层反馈事务执行结果
    unordered_map<int, TcpConnectionPtr> table_;

    // 记录txid和trans的对应关系
    unordered_map<int, struct transaction *> txidTrans_;

    ThreadPool *threadPool_;
    Dbtm *dbtm_;
};

} // namespace grit