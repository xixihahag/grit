#include <iostream>
#include <glog/logging.h>
#include "gtm/gtm.h"
#include "configManager.h"
#include "headerCmd.h"
#include "muduo/net/TcpServer.h"
#include "flatbuffers/flatbuffers.h"
#include "muduo/net/EventLoop.h"
#include "net_generated.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace flat;
using namespace grit;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }

    LOG(INFO) << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    // 通过flatbuffers判断过来的请求是啥
    string str(buf->retrieveAllAsString());
    auto msg = GetRootMsg((uint8_t *) str.c_str());
    auto gtm = static_cast<const GtmMsg *>(msg->any());
    auto cmd = gtm->cmd();

    switch (cmd) {
    case kGetTxid:
        GTM::getInstance()->getTxid(conn, gtm->transType()->str());
        break;
    case kJudgeConflit:
        GTM::getInstance()->judgeConflict(conn, gtm);
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
        // LOG(INFO) << "init ConfigManager done" << endl;

        // 将大于等于该级别的日志同时输出到stderr。
        // 日志级别 INFO, WARNING, ERROR,FATAL 的值分别为0、1、2、3。
        FLAGS_stderrthreshold = 0;
        // FLAGS_logtostderr = true;
        // 终端输出带颜色
        FLAGS_colorlogtostderr = true;
        // 磁盘已满时不记录日志
        FLAGS_stop_logging_if_full_disk = true;
        // 设置日志位置
        FLAGS_log_dir = ConfigManager::getInstance()->logDir();

        // 初始化gtm
        GTM::getInstance()->init();

        const char *ip =
            ConfigManager::getInstance()->gtmListenAddress().c_str();
        uint16_t port =
            static_cast<uint16_t>(ConfigManager::getInstance()->gtmPort());

        InetAddress listenAddr(ip, port);
        int threadCount = ConfigManager::getInstance()->gtmThreads();

        EventLoop loop;

        TcpServer server(&loop, listenAddr, "gtm");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) { server.setThreadNum(threadCount); }

        LOG(INFO) << "init gtm done";

        server.start();

        loop.loop();
    }

    return 0;
}