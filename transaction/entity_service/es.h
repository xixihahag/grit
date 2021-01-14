#pragma once

#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"

namespace grit {

class ES
{
  public:
    ES(muduo::net::EventLoop *);

    void onDbsConnection(const muduo::net::TcpConnectionPtr &);

    // 直接将app传过来的数据进行转发
    void forward(const ESMsg *);

    // 将数据处理后转发，处理成带key的读写集的形式
    void solve(const ESMsg *);

  private:
    // muduo::net::EventLoop *loop_;
    muduo::net::TcpConnectionPtr dbsConn_;
};

} // namespace grit