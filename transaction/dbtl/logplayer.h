#pragma once
#include "muduo/net/TcpConnection.h"
#include "dbtl.h"
#include "timeWheel.h"
#include <list>
#include <vector>
#include <unordered_map>
#include <vector>

namespace grit {

class Dbtl;

class LogPlayer
{
  public:
    // 用于连接底层存储服务器
    void init(Dbtl *, muduo::net::EventLoop *);

    // 用于进行日志的分发
    void playLog(int);

    // 用于接收db关于事务是否执行成功的回应
    void solve(const flat::LogPlayerMsg *);

  private:
    void onDbConnection(const muduo::net::TcpConnectionPtr &);

    // 保存所有db的连接
    std::vector<muduo::net::TcpConnectionPtr> dbConnVec_;

    Dbtl *dbtl_;

    // 存储谁应答了，谁没应答
    // txid --> dbConnList对应的下表，存储的是没应答的下标
    unordered_map<int, list<int> > answerTable_;

    // txid->pos->retryTime 重试次数表
    unordered_map<int, vector<int> > retryTable_;

    // 开启时间轮
    TimeWheel *timeWheel_;
};

} // namespace grit