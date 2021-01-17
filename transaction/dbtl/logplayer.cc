#include "logplayer.h"
#include "net_generated.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
#include <vector>

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

void LogPlayer::init(Dbtl *dbtl, EventLoop *loop)
{
    dbtl_ = dbtl;

    const char *ip = ConfigManager::getInstance()->dbtlAddress().c_str();
    uint16_t port =
        static_cast<uint16_t>(ConfigManager::getInstance()->dbtlPort());
    InetAddress servAddr(ip, port);
    TcpClient *dbClient_ = new TcpClient(loop, servAddr, "db");
    dbClient_->connect();
    dbClient_->setConnectionCallback(
        bind(&onDbConnection, this, std::placeholders::_1));
}

void LogPlayer::onDbConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }
    dbConnList_.emplace_back(conn);
}

void LogPlayer::playLog(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<Data> > data_vec;

    for (auto log : dbtl_->logTable_[txid]) {
        auto key = builder.CreateString(log->key);
        auto attr = builder.CreateString(log->attribute);
        auto val = builder.CreateString(log->value);
        auto data = CreateData(builder, key, attr, val);

        data_vec.emplace_back(data);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto db = CreateDbMsg(builder, txid, data_data);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    for (auto conn : dbConnList_)
        conn->send(ptr, size);
}

void LogPlayer::solve() {}