#include <iostream>
#include "gtm.h"
#include "muduo/net/TcpServer.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffer/net_generated.h"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace flat;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }

    // TODO: 设置glog
    // LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
    //          << conn->localAddress().toIpPort() << " is "
    //          << (conn->connected() ? "UP" : "DOWN");
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp)
{
    // conn->send(buf);
    // TODO: 通过flatbuffers判断过来的请求是啥
    auto msg = GetTest((uint8_t *) buf->retrieveAllAsString().c_str());
    auto t1 = msg->t1();
    auto t2 = msg->t2();
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    } else {
        // LOG_INFO << "pid = " << getpid() << ", tid = " <<
        // CurrentThread::tid(); Logger::setLogLevel(Logger::WARN);

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