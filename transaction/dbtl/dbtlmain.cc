#include <iostream>
#include <glog/logging.h>
#include "dbtl.h"
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

grit::Dbtl *dbtl;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }

    LOG(INFO) << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    string str(buf->retrieveAllAsString());
    auto msg = GetRootMsg((uint8_t *) str.c_str());

    if (msg->any_type() == Msg_DbtlMsg) {
        auto data = static_cast<const DbtlMsg *>(msg->any());

        // 全部转发到dbtl处理
        auto cmd = data->cmd();

        switch (cmd) {
        case kLog:
        case kLsn:
            dbtl->solve(conn, data);
            break;
        default:
            LOG(ERROR) << "receive error cmd";
        }
    } else if (msg->any_type() == Msg_LogPlayerMsg) {
        auto lp = static_cast<const LogPlayerMsg *>(msg->any());
        auto cmd = lp->cmd();
        switch (cmd) {
        case kExecTranSucc:
        case kExecTranFail:
            dbtl->logPlayer_->solve(lp);
            break;
        default:
            LOG(ERROR) << "receive error cmd";
        }
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

        // 初始化dbtl
        dbtl = new grit::Dbtl(&loop);

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