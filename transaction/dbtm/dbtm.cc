#include "dbtm.h"
#include "headerCmd.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
#include <glog/logging.h>

using namespace grit;
using namespace std;
using namespace flat;
using namespace muduo;
using namespace muduo::net;

void Dbtm::onDbtlConnection(const muduo::net::TcpConnectionPtr &conn)
{
    LOG(INFO) << "connect tbtl success";
    dbtlConn_ = conn;
}
void Dbtm::onGtmConnection(const muduo::net::TcpConnectionPtr &conn)
{
    LOG(INFO) << "connect gtm success";
    gtmConn_ = conn;
}

void Dbtm::init(EventLoop *loop)
{
    // 连接dbtl
    const char *ip = ConfigManager::getInstance()->dbtlAddress().c_str();
    uint16_t port =
        static_cast<uint16_t>(ConfigManager::getInstance()->dbtlPort());
    InetAddress servAddr(ip, port);
    TcpClient *dbtlClient_ = new TcpClient(loop, servAddr, "dbtl");
    dbtlClient_->connect();
    dbtlClient_->setConnectionCallback(bind(&onDbtlConnection, this, _1));

    // 连接gtm
    ip = ConfigManager::getInstance()->gtmAddress().c_str();
    port = static_cast<uint16_t>(ConfigManager::getInstance()->gtmPort());
    InetAddress servAddr(ip, port);
    TcpClient *gtmClient_ = new TcpClient(loop, servAddr, "gtm");
    gtmClient_->connect();
    gtmClient_->setConnectionCallback(bind(&onGtmConnection, this, _1));
}

void Dbtm::judgeLocalConflict(struct transaction *trans)
{
    auto lsn = trans->lsn;

    bool haveConflict = false;
    for (auto it = rcheck.begin(); it != rcheck.end() && !haveConflict;) {
        if (it->first > lsn) {
            for (auto rs : trans->readSet)
                if (it->second.find(rs->key) != it->second.end()) {
                    haveConflict = true;
                    break;
                }
            it++;
        } else
            it = rcheck.erase(it);
    }

    for (auto it = wcheck.begin(); it != wcheck.end() && !haveConflict;) {
        if (it->first > lsn) {
            for (auto ws : trans->writeSet)
                if (it->second.find(ws->key) != it->second.end()) {
                    haveConflict = true;
                    break;
                }
            it++;
        } else
            it = wcheck.erase(it);
    }

    if (!haveConflict) {
        table[trans->txid] = trans;
        getLsnAndGlobalConflict(trans->txid);
    }
}

void Dbtm::getLsnAndGlobalConflict(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    auto logstore = CreateLogStore(builder, kLsn, txid);
    builder.Finish(logstore);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);

    flatbuffers::FlatBufferBuilder builder1;
    auto gtm = CreateGtm(builder1, kJudgeConflit, txid);
    builder.Finish(gtm);

    ptr = (char *) builder1.GetBufferPointer();
    size = builder1.GetSize();

    gtmConn_->send(ptr, size);
}

void Dbtm::solve(const flat::DbService *data)
{
    auto type = data->type();
    auto txid = data->txid();
    struct transaction *trans = table[txid];

    switch (type) {
    case kJudgeConflit:
        if (!(data->isConflict())) trans->isConflict = false;
        break;
    case kLsn:
        trans->lsn = data->lsn();
    }

    if (!(trans->isConflict) && trans->lsn.size() > 0) cacheRWSet(trans);
}

void Dbtm::cacheRWSet(struct transaction *trans)
{
    string lsn = trans->lsn;

    rcheck[lsn].emplace(trans->trcheck);
    wcheck[lsn].emplace(trans->twcheck);

    sendLog();
}

void Dbtm::sendLog() {}