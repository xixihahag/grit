#include <iostream>
#include <glog/logging.h>
#include "configManager.h"
#include "headerCmd.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "flatbuffers/flatbuffers.h"
#include "net_generated.h"
#include "es.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace flat;
using namespace grit;

ES *es;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }

    LOG(INFO) << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    string str(buf->retrieveAllAsString());
    auto msg = GetRootMsg((uint8_t *) str.c_str());
    auto data = static_cast<const ESMsg *>(msg->any());
    auto cmd = data->cmd();

    es->table_[data->txid()] = conn;

    switch (cmd) {
    case kAdd:
    case kDelete:
    case kChange:
    case kSearch:
        // FIXME:
        // 讲道理这里应该做一些业务上的处理，转换成对应的sql语句，解析sql再转发过去的
        // 为了演示简单，逻辑完整，这里直接转换成读写集的形式再发送给dbs
        es->solve(data);
        break;
    case kCommit:
        es->forward(data);
        break;
    case kTranSuccess:
    case kTranFail:
        break;
    default:
        LOG(ERROR) << "receive error cmd";
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
        es = new ES(&loop);

        const char *ip =
            ConfigManager::getInstance()->esListenAddress().c_str();
        uint16_t port =
            static_cast<uint16_t>(ConfigManager::getInstance()->esPort());

        InetAddress listenAddr(ip, port);
        // int threadCount = ConfigManager::getInstance()->dbsThreads();

        TcpServer server(&loop, listenAddr, "es");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        // if (threadCount > 1) { server.setThreadNum(threadCount); }

        LOG(INFO) << "init es done";

        server.start();

        loop.loop();
    }

    return 0;
}