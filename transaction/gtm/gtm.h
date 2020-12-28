#pragma once

#include <atomic>
#include <unordered_map>
#include <map>
#include <queue>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "singleton.h"
namespace grit {
class GTM : public Singleton<GTM>
{
  public:
    void getTxid(const muduo::net::TcpConnectionPtr &);
    void judgeConflict();

    // 读取文件，缓存每个事务涉及的DB在哪
    void init();

  private:
    std::atomic<int> txid_;

    void onConnection(const muduo::net::TcpConnectionPtr &);
    void onMessage(
        const muduo::net::TcpConnectionPtr &,
        muduo::net::Buffer *,
        muduo::Timestamp);

    struct IpandPort
    {
        IpandPort(std::string ip, int port)
            : ip_(ip)
            , port_(port)
        {}
        std::string ip_;
        int port_;
    };

    std::unordered_map<std::string, std::queue<struct IpandPort *> > transInfo_;
};

} // namespace grit