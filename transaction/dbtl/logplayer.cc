#include "logplayer.h"
#include "net_generated.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
#include "headerCmd.h"
#include <vector>
#include <glog/logging.h>

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
    dbConnVec_.emplace_back(conn);
}

void LogPlayer::playLog(int txid)
{
    // 将服务器下表添加进未应答队列里面
    if (answerTable_.find(txid) == answerTable_.end())
        for (int i = 0; i < dbConnVec_.size(); i++)
            answerTable_[txid].emplace_back(i);

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

    for (auto pos : answerTable_[txid])
        dbConnVec_[pos]->send(ptr, size);
}

void LogPlayer::solve(const LogPlayerMsg *data)
{
    auto cmd = data->cmd();

    switch (cmd) {
    case kExecTranSucc:
        answerTable_[data->txid()].remove(data->id());
        break;
    case kExecTranFail:
        // TODO: 可以设置重试次数限制，或者设置定时器，多久之后重试
        // 针对每一个txid的每一个conn，设置定时器，设置重试次数，设置重试间隔时间
        playLog(data->txid());
        break;
    default:
        LOG(ERROR) << "receive error cmd";
    }
}