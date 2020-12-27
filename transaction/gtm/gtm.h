#pragma once

#include <atomic>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "singleton.h"

namespace grit {

class GTM : public Singleton<GTM>
{
  public:
    void getTxid(const muduo::net::TcpConnectionPtr &);
    void judgeConflict();

  private:
    std::atomic<int> txid_;

    void onConnection(const muduo::net::TcpConnectionPtr &);
    void onMessage(
        const muduo::net::TcpConnectionPtr &,
        muduo::net::Buffer *,
        muduo::Timestamp);
};

} // namespace grit