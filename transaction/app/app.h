#pragma once
#include "muduo/net/TcpConnection.h"
#include "muduo/net/EventLoop.h"
#include "net_generated.h"
#include <unordered_map>

namespace grit {

class App
{
  public:
    // 与gtm建立连接
    App(EventLoop *);

    // 开启一个事务
    void startTran();

    // 连接需要的dbs
    void connect2Dbs(
        int,
        const flatbuffers::Vector<flatbuffers::Offset<flat::ipAndPort> > *);

  private:
    void onGtmConnection(const muduo::net::TcpConnectionPtr &);
    void onDbsConnection(const muduo::net::TcpConnectionPtr &);

    // 维护gtm的连接
    muduo::net::TcpConnectionPtr gtmConn_;

    EventLoop *loop_;

    // ip和txid对应表
    std::unordered_map<std::string, int> table_;
};

} // namespace grit