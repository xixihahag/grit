#pragma once
#include "net_generated.h"
#include "dbservice/dbservice.h"
#include "muduo/net/EventLoop.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace grit {

class Dbtm
{
  public:
    // 初始化，用于连接LogStore服务器和GTM服务器
    void init(EventLoop *);

    // 用于处理lsn和gtm冲突处理的结果
    void solve(const flat::DbService *);

    // 用于判断本地是否存在冲突
    void judgeLocalConflict(struct transaction *);

  private:
    // 获取lsn并且判断事务是否存在全局冲突
    void getLsnAndGlobalConflict(int);

    // 缓存当前事务的读写集
    void cacheRWSet(struct transaction *);

    // 发送日志给LogStore
    void sendLog();

    // 连接之后的回调
    void onDbtlConnection(const muduo::net::TcpConnectionPtr &);
    void onGtmConnection(const muduo::net::TcpConnectionPtr &);

    // 用于快速进行数据冲突验证
    // TODO: 数据验证好像可以用位图来做，参考底层的分布式图数据库存储
    std::unordered_map<std::string, std::unordered_set<std::string> > rcheck,
        wcheck;

    // 用于txid和对应事务的映射
    std::unordered_map<int, struct transaction *> table;

    // 连接dbtl和gtm
    TcpConnectionPtr dbtlConn_, gtmConn_;
};

} // namespace grit