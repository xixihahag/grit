#include <iostream>
#include <glog/logging.h>
#include "gtm/gtm.h"
#include "configManager.h"
#include "headerCmd.h"
#include "muduo/net/TcpServer.h"
#include "flatbuffers/flatbuffers.h"
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
    // conn->send(buf);
    // TODO: 通过flatbuffers判断过来的请求是啥
    auto msg = GetRootMsg((uint8_t *) buf->retrieveAllAsString().c_str());
    auto gtm = static_cast<const Gtm *>(msg->any());
    auto type = gtm->type();

    switch (type) {
    case kGetTxid:
        GTM::getInstance()->getTxid(conn);
        break;
    case kJudgeConflit:
        GTM::getInstance()->judgeConflict();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: server <config dir>\n");
    } else {
        // LOG_INFO << "pid = " << getpid() << ", tid = " <<
        // CurrentThread::tid(); Logger::setLogLevel(Logger::WARN);

        google::InitGoogleLogging(argv[0]);

        // 初始化配置信息
        ConfigManager::getInstance()->init(argv[1]);

        // 将大于等于该级别的日志同时输出到stderr。
        // 日志级别 INFO, WARNING, ERROR,FATAL 的值分别为0、1、2、3。
        FLAGS_stderrthreshold = 2;
        // 设置日志位置
        FLAGS_log_dir = ConfigManager::getInstance()->logDir();

        // Glog库还提供了一个信号处理器
        // 能够在 SIGSEGV之类的信号导致的程序崩溃时导出有用的信息。
        google::InstallFailureSignalHandler();

        const char *ip = ConfigManager::getInstance()->address().c_str();
        uint16_t port =
            static_cast<uint16_t>(ConfigManager::getInstance()->port());
        InetAddress listenAddr(ip, port);
        int threadCount = ConfigManager::getInstance()->threads();

        EventLoop loop;

        TcpServer server(&loop, listenAddr, "gtm");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) { server.setThreadNum(threadCount); }

        server.start();

        loop.loop();
    }

    return 0;
}