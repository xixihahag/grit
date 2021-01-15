#include "dbtm.h"
#include "headerCmd.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
// #include "rocksdb/write_batch.h"
#include "flatbuffer/net_generated.h"
#include <glog/logging.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace grit;
using namespace std;
using namespace flat;
using namespace muduo;
using namespace muduo::net;
// using namespace ROCKSDB_NAMESPACE;

Dbtm::Dbtm(DbService *dbs) { dbservice_ = dbs; }

Dbtm::~Dbtm()
{
    // delete rocksDb_;
}

void Dbtm::onDbtlConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }
    LOG(INFO) << "connect tbtl success";
    dbtlConn_ = conn;
}
void Dbtm::onGtmConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (conn->connected()) { conn->setTcpNoDelay(true); }
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

    // 开启落盘线程池
    threadPool_ = new ThreadPool(1);
}

void Dbtm::judgeLocalConflict(struct transaction *trans)
{
    auto lsn = trans->lsn;
    bool haveConflict = false;

    // 检查读写冲突
    for (auto it : trans->readSet) {
        wcheck.delLessThan(it->key, trans->lsn);
        if (wcheck.exist(it->key)) {
            haveConflict = true;
            break;
        }
    }

    // 检查写写冲突
    for (auto it : trans->writeSet) {
        if (haveConflict) break;
        wcheck.delLessThan(it->key, trans->lsn);
        if (wcheck.exist(it->key)) {
            haveConflict = true;
            break;
        }
    }

    if (!haveConflict) {
        table_[trans->txid] = trans;
        dbservice_->txidTrans_.erase(trans->txid);
        if (trans->needGlobalConflct)
            judgeGlobalConflict(trans->txid);
        else
            cacheRWSet(trans);
    } else {
        LOG(INFO) << "There are conflicts between transactions (local)";
        dbservice_->tranFail(trans->txid);
    }
}

// void Dbtm::getLsnAndGlobalConflict(int txid)
// {
//     flatbuffers::FlatBufferBuilder builder;
//     auto dbtl = CreateDbtlMsg(builder, kLsn, txid);
//     builder.Finish(dbtl);

//     char *ptr = (char *) builder.GetBufferPointer();
//     uint64_t size = builder.GetSize();

//     dbtlConn_->send(ptr, size);

//     flatbuffers::FlatBufferBuilder builder1;
//     auto gtm = CreateGtmMsg(builder1, kJudgeConflit, txid);
//     builder.Finish(gtm);

//     ptr = (char *) builder1.GetBufferPointer();
//     size = builder1.GetSize();

//     gtmConn_->send(ptr, size);
// }

void Dbtm::getLsn(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    auto dbtl = CreateDbtlMsg(builder, kLsn, txid);
    builder.Finish(dbtl);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);
}

void Dbtm::judgeGlobalConflict(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    auto gtm = CreateGtmMsg(builder, kJudgeConflit, txid);
    builder.Finish(gtm);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    gtmConn_->send(ptr, size);
}

void Dbtm::solve(const DbServiceMsg *data)
{
    auto cmd = data->cmd();

    switch (cmd) {
    // 全局判冲突结果返回
    case kJudgeConflit:
        if (!(data->isGlobalConflict()))
            cacheRWSet(table_[data->txid()]);
        else {
            LOG(INFO) << "There are conflicts between transactions (global)";
            dbservice_->tranFail(data->txid());
        }
        break;
    }
}

void Dbtm::cacheRWSet(struct transaction *tran)
{
    int lsn = tran->lsn;

    for (auto it : tran->writeSet)
        wcheck.insert(it->key, lsn);

    sendLog(tran);
}

// 开个线程专门做这事，给个batch做send
// FIXME: 数据库需要做主备切换么
// TODO: 要不不用rocksDB，直接用文件存储传输，日志可以做成幂等
void Dbtm::writeToDiskByRocksDB(struct transaction *tran)
{
    // // TODO: 看看这句话应该放在哪
    // threadPool_->enqueue(bind(&Dbtm::writeToDisk, this, tran));

    // // 以 标签#属性:值@ 的形式定义一条数据进行落盘
    // WriteBatch batch;
    // for (auto data : tran->writeSet) {
    //     string val;
    //     val += data->attribute;
    //     val += ':';
    //     val += data->value;
    //     val += '@';

    //     batch.Put(data->label, val);
    // }
    // Status s = rocksDb_->Write(WriteOptions(), &batch);
    // if (!s.ok()) LOG(ERROR) << "insert into rocksDB error";

    // // TODO: 通知app事务执行成功
}

// TODO: 这样落盘很慢，不行找个东西优化下
// 以dbtl消息的形式将数据落盘，判断一下什么时候落盘，是否落盘
void Dbtm::writeToDisk(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        // auto label = builder.CreateString(data->label);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtlMsg(builder, kData, tran->txid, data_data);
    builder.Finish(dbtl);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    // 落盘
    int fd = open(
        to_string(tran->txid).c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) LOG(ERROR) << "open log file error";

    write(fd, ptr, size);
    close(fd);

    // 通知app事务执行成功
    flatbuffers::FlatBufferBuilder builder;
    auto app = CreateAppMsg(builder, kTranSuccess);
    builder.Finish(app);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbservice_->table_[tran->txid]->send(ptr, size);
}

void Dbtm::sendLog(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        // auto label = builder.CreateString(data->label);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtlMsg(builder, kData, tran->txid, data_data);
    builder.Finish(dbtl);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);
}

void Dbtm::sendLogByDisk(string &fileName) {}