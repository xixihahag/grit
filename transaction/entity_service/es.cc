#include "es.h"
#include "net_generated.h"
#include "configManager.h"
#include "headerCmd.h"
#include "muduo/net/TcpClient.h"
#include <vector>

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo;
using namespace muduo::net;

ES::ES(EventLoop *loop)
{
    // 连接dbs
    const char *ip = ConfigManager::getInstance()->dbsAddress().c_str();
    uint16_t port =
        static_cast<uint16_t>(ConfigManager::getInstance()->dbsPort());
    InetAddress servAddr(ip, port);

    TcpClient *dbsClient_ = new TcpClient(loop, servAddr, "dbs");
    dbsClient_->connect();
    dbsClient_->setConnectionCallback(
        bind(&onDbsConnection, this, std::placeholders::_1));
}

void ES::onDbsConnection(const muduo::net::TcpConnectionPtr &conn)
{
    dbsConn_ = conn;
}

void ES::forward(const ESMsg *data)
{
    flatbuffers::FlatBufferBuilder builder;

    auto dbs = CreateDbServiceMsg(builder, kCommit, data->txid());
    builder.Finish(dbs);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbsConn_->send(ptr, size);
}

void ES::solve(const ESMsg *msg)
{
    flatbuffers::FlatBufferBuilder builder;

    auto key = builder.CreateString(msg->key()->str());
    flatbuffers::Offset<flat::Data> data;
    vector<flatbuffers::Offset<flat::Data> > data_vec;
    flatbuffers::Offset<flat::DbServiceMsg> dbs;

    if (msg->cmd() == kAdd || msg->cmd() == kDelete) {
        auto attr = builder.CreateString(msg->attr()->str());
        auto val = builder.CreateString(msg->val()->str());

        data = CreateData(builder, key, attr, val);
        data_vec.push_back(data);
        auto data_data = builder.CreateVector(data_vec);
        dbs = CreateDbServiceMsg(
            builder, kData, msg->txid(), false, false, 0, 0, data_data);

    } else {
        data = CreateData(builder, key);
        data_vec.push_back(data);
        auto data_data = builder.CreateVector(data_vec);
        dbs = CreateDbServiceMsg(
            builder, kData, msg->txid(), false, false, 0, data_data, 0);
    }

    builder.Finish(dbs);
    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbsConn_->send(ptr, size);
}