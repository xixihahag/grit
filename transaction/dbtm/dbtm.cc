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

    // 连接本地mysql
    string host = "127.0.0.1";
    string user = ConfigManager::getInstance()->dbtmUser();
    string passwd = ConfigManager::getInstance()->dbtmPasswd();
    string dbName = ConfigManager::getInstance()->dbtmDbName();
    int port = ConfigManager::getInstance()->dbtmDbPort();

    bool conn = db_.initDB(host, user, passwd, dbName, port);
    if (!conn) LOG(ERROR) << "connect mysql failed";
    initMySql();
}

void Dbtm::initMySql()
{
    string sql = "CREATE TABLE IF NOT EXISTS `transInfo`( \
        `id` INT UNSIGNED AUTO_INCREMENT,   \
        `txid` INT UNSIGNED NOT NULL,   \
        `lsn` int,  \
        `needLocalConflict` bool,   \
        `needGlobalConflict` bool,  \
        `fileName` VARCHAR(40),  \
        PRIMARY KEY(`id`)  \
    )ENGINE=InnoDB DEFAULT CHARSET=UTF8;";

    sqlExec(sql);
}

void Dbtm::judgeLocalConflict(struct transaction *trans)
{
    // 落盘并且改写sql
    writeToDiskAndSql(trans);

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
        sqlUpdateNeedLocalConflict(trans->txid, false);
        if (trans->needGlobalConflct)
            judgeGlobalConflict(trans->txid);
        else
            cacheRWSet(trans);
    } else {
        LOG(INFO) << "There are conflicts between transactions (local)";
        dbservice_->retResult(kTranFail, trans->txid);

        if (trans->needGlobalConflct) {
            // 给GTM发送冲突结果
            flatbuffers::FlatBufferBuilder builder;
            auto gtm = CreateGtmMsg(builder, kJudgeConflit, trans->txid, true);

            char *ptr = (char *) builder.GetBufferPointer();
            uint64_t size = builder.GetSize();

            gtmConn_->send(ptr, size);
        }
    }
}

void Dbtm::getLsn(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    auto dbtl = CreateDbtlMsg(builder, kLsn, txid);
    auto msg = CreateRootMsg(builder, Msg_DbtlMsg, dbtl.Union());
    builder.Finish(msg);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);
}

void Dbtm::judgeGlobalConflict(int txid)
{
    flatbuffers::FlatBufferBuilder builder;
    auto gtm = CreateGtmMsg(builder, kJudgeConflit, txid);
    auto msg = CreateRootMsg(builder, Msg_GtmMsg, gtm.Union());
    builder.Finish(msg);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    gtmConn_->send(ptr, size);
}

void Dbtm::solve(const DbtmMsg *data)
{
    auto cmd = data->cmd();

    switch (cmd) {
    // 全局判冲突结果返回
    case kJudgeConflit:
        if (!(data->isGlobalConflict())) {
            cacheRWSet(table_[data->txid()]);
            sqlUpdateNeedGlobalConflict(data->txid(), false);
        } else {
            LOG(INFO) << "There are conflicts between transactions (global)";
            sqlDelete(data->txid());
            dbservice_->retResult(kTranFail, data->txid());
        }
        break;
    }
}

void Dbtm::cacheRWSet(struct transaction *tran)
{
    int lsn = tran->lsn;

    for (auto it : tran->writeSet)
        wcheck.insert(it->key, lsn);

    // writeToDisk(tran);
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
// 以dbtl消息的形式将数据落盘
void Dbtm::writeToDiskAndSql(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtlMsg(builder, kData, tran->txid, data_data);
    auto msg = CreateRootMsg(builder, Msg_DbtlMsg, dbtl.Union());
    builder.Finish(msg);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    // 落盘
    string filePath =
        ConfigManager::getInstance()->dbtmLogPath() + to_string(tran->txid);
    int fd = open(filePath.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) LOG(ERROR) << "open log file error";

    write(fd, ptr, size);
    close(fd);

    // 写入sql
    sqlInsert(tran->txid, tran->lsn, to_string(tran->txid).c_str());
}

void Dbtm::sendLog(struct transaction *tran)
{
    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<flat::Data> > data_vec;

    for (auto data : tran->writeSet) {
        auto key = builder.CreateString(data->key);
        auto attr = builder.CreateString(data->attribute);
        auto val = builder.CreateString(data->value);

        auto wData = CreateData(builder, key, attr, val);
        data_vec.push_back(wData);
    }

    auto data_data = builder.CreateVector(data_vec);
    auto dbtl = CreateDbtlMsg(builder, kData, tran->txid, data_data);
    auto msg = CreateRootMsg(builder, Msg_DbtlMsg, dbtl.Union());
    builder.Finish(msg);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    dbtlConn_->send(ptr, size);
}

void Dbtm::sendLogByDisk(string &fileName) {}

void Dbtm::sqlExec(std::string &sql)
{
    if (db_.exeSQL(sql) == false)
        LOG(WARNING) << "mysql insert to tranInfo failed";
}

void Dbtm::sqlInsert(int txid, int lsn, string fileName)
{
    string sql =
        "INSERT INTO transInfo(txid,lsn,needLocalConflict,needGlobalConflict,fileName) \
    VALUES (";
    sql += to_string(txid);
    sql += ',';
    sql += to_string(lsn);
    sql += ', true, true,';
    sql += fileName;
    sql += ");";

    sqlExec(sql);
}
void Dbtm::sqlDelete(int txid)
{
    string sql = "DELETE FROM transInfo \
        where txid = ";
    sql += txid;
    sql += ";";

    sqlExec(sql);

    // 顺便把本地文件也删了
    string filePath =
        ConfigManager::getInstance()->dbtmLogPath() + to_string(txid);
    if (remove(filePath.c_str()) != 0)
        LOG(WARNING) << "delete local file failed";
}

void Dbtm::sqlUpdateNeedLocalConflict(int txid, bool status)
{
    // 这里直接把值置为false的原因是，如果存在冲突则不可能调用此函数，会直接删掉此项
    // 由此可知，如果调用此函数则一定不存在冲突，所以把值置为false
    // 下同
    string sql = "UPDATE transInfo SET needLocalConflict=false WHERE txid=";
    sql += txid;
    sql += ";";

    sqlExec(sql);
}
void Dbtm::sqlUpdateNeedGlobalConflict(int txid, bool status)
{
    string sql = "UPDATE transInfo SET needGlobalConflict=false WHERE txid=";
    sql += txid;
    sql += ";";

    sqlExec(sql);
}