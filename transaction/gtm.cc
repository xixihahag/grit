#include <iostream>
#include <glog/logging.h>
#include "gtm.h"
#include "configManager.h"
#include "muduo/net/TcpServer.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffer/net_generated.h"

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
    auto msg = GetTest((uint8_t *) buf->retrieveAllAsString().c_str());
    auto t1 = msg->t1();
    auto t2 = msg->t2();
}

int GTM::getTxid() { return txid_.load(); }

bool GTM::judgeConflict()
{
    // TODO: 待定
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    } else {
        // LOG_INFO << "pid = " << getpid() << ", tid = " <<
        // CurrentThread::tid(); Logger::setLogLevel(Logger::WARN);

        google::InitGoogleLogging(argv[0]);

        // 将大于等于该级别的日志同时输出到stderr。
        // 日志级别 INFO, WARNING, ERROR,FATAL 的值分别为0、1、2、3。
        FLAGS_stderrthreshold = 2;
        // 设置日志位置
        FLAGS_log_dir = ConfigManager::getInstance()->logDir();

        const char *ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TcpServer server(&loop, listenAddr, "PingPong");

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) { server.setThreadNum(threadCount); }

        server.start();

        loop.loop();
    }

    return 0;
}