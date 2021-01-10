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

    // // 打开rocksDB
    // // Optimize RocksDB. This is the easiest way to get RocksDB to perform
    // well rockesDBOptions_.IncreaseParallelism();
    // rockesDBOptions_.OptimizeLevelStyleCompaction();
    // // create the DB if it's not already present
    // rockesDBOptions_.create_if_missing = true;
    // // open DB
    // Status s = DB::Open(
    //     rockesDBOptions_,
    //     ConfigManager::getInstance()->dbtmRocksDbPath(),
    //     &rocksDb_);
    // if (!s.ok()) LOG(ERROR) << "open rocksDB error";

    // 开启落盘线程池
    threadPool_ = new ThreadPool(1);
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
    auto dbtl = CreateDbtlMsg(builder, kLsn, txid);
    builder.Finish(dbtl);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);

    flatbuffers::FlatBufferBuilder builder1;
    auto gtm = CreateGtmMsg(builder1, kJudgeConflit, txid);
    builder.Finish(gtm);

    ptr = (char *) builder1.GetBufferPointer();
    size = builder1.GetSize();

    gtmConn_->send(ptr, size);
}

void Dbtm::solve(const DbServiceMsg *data)
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

void Dbtm::cacheRWSet(struct transaction *tran)
{
    string lsn = tran->lsn;

    rcheck[lsn].emplace(tran->trcheck);
    wcheck[lsn].emplace(tran->twcheck);

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
void Dbtm::writeToDisk(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        auto label = builder.CreateString(data->label);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, label, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtl(builder, kData, tran->txid, data_data);
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
    auto app = CreateApp(builder, kTranSuccess);
    builder.Finish(app);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbservice_->table[tran->txid]->send(ptr, size);
}

void Dbtm::sendLog(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        auto label = builder.CreateString(data->label);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, label, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtl(builder, kData, tran->txid, data_data);
    builder.Finish(dbtl);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);
}

void Dbtm::sendLogByDisk(string &fileName) {}