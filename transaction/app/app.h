#pragma once
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoop.h"
#include "net_generated.h"
#include <unordered_map>
#include <list>

namespace grit {

class App
{
  public:
    // 与gtm建立连接
    App(EventLoop *);

    // 开启一个事务
    void startTran(std::string &);

    // 连接需要的dbs
    void connect2ES(
        const flatbuffers::Vector<flatbuffers::Offset<flat::ipAndPort> > *);

    // 添加事务语句进来
    void add(std::string);

    // 提交事务
    void commit();

    // 负责传输协议的解析
    void onMessage(const TcpConnectionPtr &, Buffer *, muduo::Timestamp);

  private:
    void onGtmConnection(const muduo::net::TcpConnectionPtr &);
    void onESConnection(const muduo::net::TcpConnectionPtr &);

    // 维护gtm的连接
    muduo::net::TcpConnectionPtr gtmConn_;

    EventLoop *loop_;

    int txid_;
    list<muduo::net::TcpConnectionPtr> connList_;
};

} // namespace grit