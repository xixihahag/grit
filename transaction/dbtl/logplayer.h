#pragma once
#include "muduo/net/TcpConnection.h"
#include "dbtl.h"
#include <list>

namespace grit {

class LogPlayer
{
  public:
    // 用于连接底层存储服务器
    void init(Dbtl *, EventLoop *);

    // 用于进行日志的分发
    void playLog(int);

    // TODO: 用于接收db关于事务是否执行成功的回应
    void solve();

  private:
    void onDbConnection(const muduo::net::TcpConnectionPtr &);

    std::list<muduo::net::TcpConnectionPtr> dbConnList_;

    Dbtl *dbtl_;

    // TODO: 怎么存储谁应答了，谁没应答哇
};

} // namespace grit