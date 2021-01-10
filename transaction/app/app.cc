#include "app.h"
#include "headerCmd.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

App ::App(EventLoop *loop)
{
    loop_ = loop;

    const char *ip = ConfigManager::getInstance()->gtmAddress().c_str();
    uint16_t port =
        static_cast<uint16_t>(ConfigManager::getInstance()->gtmPort());
    InetAddress servAddr(ip, port);
    TcpClient *gtmClient_ = new TcpClient(loop, servAddr, "gtm");
    gtmClient_->connect();
    gtmClient_->setConnectionCallback(
        bind(&onGtmConnection, this, std::placeholders::_1));
}

void App::onGtmConnection(const muduo::net::TcpConnectionPtr &conn)
{
    gtmConn_ = conn;
}

void App::connect2Dbs(
    int txid,
    const flatbuffers::Vector<flatbuffers::Offset<flat::ipAndPort> > *data)
{
    int size = data->size();
    for (int i = 0; i < size; i++) {
        auto ipandport = data->Get(i);

        string ip = ipandport->ip()->str();
        int port = ipandport->port();

        table_[ip] = txid;

        InetAddress servAddr(ip, port);
        TcpClient *dbsClient_ = new TcpClient(loop_, servAddr, "dbs");
        dbsClient_->connect();
        dbsClient_->setConnectionCallback(
            bind(&onDbsConnection, this, std::placeholders::_1));
    }
}

void App::onDbsConnection(const muduo::net::TcpConnectionPtr &conn)
{
    flatbuffers::FlatBufferBuilder builder;

    auto dbs = CreateDbServiceMsg(
        builder, kStartTran, table_[conn->peerAddress().toIp()]);

    builder.Finish(dbs);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    conn->send(ptr, size);
}

void App::startTran()
{
    // 向gtm要txid和dbs列表
    flatbuffers::FlatBufferBuilder builder;
    auto trantype = builder.CreateString("buy");
    auto gtm = CreateGtmMsg(builder, kGetTxid, trantype);
    builder.Finish(gtm);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    gtmConn_->send(ptr, size);
}