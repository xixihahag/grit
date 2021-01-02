#pragma once

#include <atomic>
#include <unordered_map>
#include <map>
#include <list>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "singleton.h"
namespace grit {
class GTM : public Singleton<GTM>
{
  public:
    // 返回TXID给app
    void getTxid(const muduo::net::TcpConnectionPtr &, std::string);

    // 判断是否存在全局冲突
    void judgeConflict();

    // 读取文件，缓存每个事务涉及的DB在哪
    void init();

  private:
    std::atomic<int> txid_;

    // void onConnection(const muduo::net::TcpConnectionPtr &);
    // void onMessage(
    //     const muduo::net::TcpConnectionPtr &,
    //     muduo::net::Buffer *,
    //     muduo::Timestamp);

    // 存储每个事务类型对应着的dbservice的ip和端口
    struct IpandPort
    {
        IpandPort(std::string ip, int port)
            : ip_(ip)
            , port_(port)
        {}
        std::string ip_;
        int port_;
    };

    std::unordered_map<std::string, std::list<struct IpandPort *> > transInfo_;
};

} // namespace grit