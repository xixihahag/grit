#include <iostream>
#include <glog/logging.h>
#include "dbservice/dbservice.h"
#include "configManager.h"
#include "headerCmd.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "flatbuffers/flatbuffers.h"
#include "net_generated.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace flat;
using namespace grit;

grit::DbService *db;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }

    LOG(INFO) << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    auto msg = GetRootMsg((uint8_t *) buf->retrieveAllAsString().c_str());
    auto data = static_cast<const DbServiceMsg *>(msg->any());
    auto cmd = data->cmd();

    // TODO: 日志发送给dbtl成功后需要告诉app已经成功执行事务

    switch (cmd) {
    case kData:
        db->getReadWriteSet(conn, data);
        if (db->txidTrans_[data->txid()]->lsn == -1) {
            db->txidTrans_[data->txid()]->lsn = 0;
            db->threadPool_->enqueue(
                bind(&Dbtm::getLsn, db->dbtm_, data->txid()));
        }
        break;
    case kCommit:
        //先检查局部冲突，然后判断是否检查全局冲突
        db->txidTrans_[data->txid()]->needGlobalConflct =
            data->needGlobalConflict();
        // 如果已经得到对应的lsn则进行冲突判断，否则置信号等lsn回来
        if (db->txidTrans_[data->txid()]->lsn > 0)
            db->threadPool_->enqueue(bind(
                &Dbtm::judgeLocalConflict,
                db->dbtm_,
                db->txidTrans_[data->txid()]));
        else
            db->txidTrans_[data->txid()]->alreadyCommit = true;
        break;
    case kJudgeConflit:
        // 全局判冲突结果返回 扔给DBTM处理
        db->threadPool_->enqueue(bind(&Dbtm::solve, db->dbtm_, data));
        break;
    case kLsn:
        db->txidTrans_[data->txid()]->lsn = data->lsn();
        if (db->txidTrans_[data->txid()]->alreadyCommit)
            db->threadPool_->enqueue(bind(
                &Dbtm::judgeLocalConflict,
                db->dbtm_,
                db->txidTrans_[data->txid()]));
        break;
    case kTranSuccess:
    case kTranFail:
        db->retResult(cmd, data->txid());
        break;
    default:
        LOG(ERROR) << "reveive error cmd";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: server <config dir>\n");
    } else {
        google::InitGoogleLogging(argv[0]);

        // Glog库还提供了一个信号处理器
        // 能够在 SIGSEGV之类的信号导致的程序崩溃时导出有用的信息。
        google::InstallFailureSignalHandler();

        // 初始化配置信息，给位置，读取配置
        ConfigManager::getInstance()->init(argv[1]);

        // 将大于等于该级别的日志同时输出到stderr。
        // 日志级别 INFO, WARNING, ERROR,FATAL 的值分别为0、1、2、3。
        FLAGS_stderrthreshold = 0;
        // 只输出到stderr
        // FLAGS_logtostderr = true;
        // 终端输出带颜色
        FLAGS_colorlogtostderr = true;
        // 磁盘已满时不记录日志
        FLAGS_stop_logging_if_full_disk = true;
        // 设置日志位置
        FLAGS_log_dir = ConfigManager::getInstance()->logDir();

        EventLoop loop;

        // 此处可以选择用带对端数据库ip的初始化方式，但方便测试使用默认构造即可
        // 初始化dbservice
        db = new grit::DbService(&loop);

        const char *ip =
            ConfigManager::getInstance()->dbsListenAddress().c_str();
        uint16_t port =
            static_cast<uint16_t>(ConfigManager::getInstance()->dbsPort());

        InetAddress listenAddr(ip, port);
        int threadCount = ConfigManager::getInstance()->dbsThreads();

        TcpServer server(&loop, listenAddr, "dbservice");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) { server.setThreadNum(threadCount); }

        LOG(INFO) << "init dbserwvice done";

        server.start();

        loop.loop();
    }

    return 0;
}