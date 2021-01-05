#include "dbtm.h"
#include "headerCmd.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
#include "rocksdb/write_batch.h"
#include <glog/logging.h>
#include <unistd.h>

using namespace grit;
using namespace std;
using namespace flat;
using namespace muduo;
using namespace muduo::net;
using namespace ROCKSDB_NAMESPACE;

Dbtm::~Dbtm() { delete rocksDb_; }

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

    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    rockesDBOptions_.IncreaseParallelism();
    rockesDBOptions_.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    rockesDBOptions_.create_if_missing = true;
    // open DB
    Status s = DB::Open(
        rockesDBOptions_,
        ConfigManager::getInstance()->rocksDbPath(),
        &rocksDb_);
    if (!s.ok()) LOG(ERROR) << "open rocksDB error";
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

// FIXME: 多线程写入rocksDB会不会有问题，多线程写入会有问题，多线程读不会
void Dbtm::writeToDisk(struct transaction *tran)
{
    // 以 标签#属性:值@ 的形式定义一条数据进行落盘

    WriteBatch batch;
    for (auto data : tran->writeSet) {
        string val;
        val += data->attribute;
        val += ':';
        val += data->value;
        val += '@';

        batch.Put(data->label, val);
    }
    Status s = rocksDb_->Write(WriteOptions(), &batch);
    if (!s.ok()) LOG(ERROR) << "insert into rocksDB error";
}

void Dbtm::sendLog()
{
    // TODO: 传一个文件过去还是传flatbuffer过去？
    // 讲道理，为了宕机不掉东西，还是得先写到本地，再传输过去，看什么时候写到本地吧
    // 可以借助rocksDB的日志进行存储，日志格式自己定义就行了
    // 先落盘，然后传文件过去

    rocksDb_->Close();
    DestroyDB(ConfigManager::getInstance()->rocksDbPath(), rockesDBOptions_);
}