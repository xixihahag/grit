#pragma once

#include <atomic>
#include <unordered_map>
#include <map>
#include <list>
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "singleton.h"
#include "net_generated.h"
namespace grit {
class GTM : public Singleton<GTM>
{
  public:
    // 返回TXID给app
    void getTxid(const muduo::net::TcpConnectionPtr &, std::string);

    // 判断是否存在全局冲突
    void
    judgeConflict(const muduo::net::TcpConnectionPtr &, const flat::GtmMsg *);

    // 读取文件，缓存每个事务涉及的DB在哪
    void init();

  private:
    // 对外提供的txid
    std::atomic<int> txid_;

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

    struct txidInfo
    {
        txidInfo(int num)
            : serverNum(num)
            , isConflict(false)
        {}
        int serverNum; // 事务对应几个服务器
        std::list<muduo::net::TcpConnectionPtr> connList_; // 服务器列表
        bool isConflict;                                   // 是否冲突
    };

    // 事务类型--对应的es的ip和端口
    std::unordered_map<std::string, std::list<struct IpandPort *> > transInfo_;

    // 存储每个事务txid需要几台服务器进行全局冲突判断
    std::unordered_map<int, struct txidInfo *> table_;
};

} // namespace grit