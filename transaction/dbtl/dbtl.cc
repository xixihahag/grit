#include "dbtl.h"
#include "base/threadPool.h"
#include "base/headerCmd.h"
#include "configManager.h"
#include "rocksdb/write_batch.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glog/logging.h>

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;
using namespace ROCKSDB_NAMESPACE;

grit::Dbtl::Dbtl(EventLoop *loop)
{
    if (ConfigManager::getInstance()->dbtlUseRocksDb()) {
        // 打开rocksDB
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform
        // well
        rockesDBOptions_.IncreaseParallelism();
        rockesDBOptions_.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        rockesDBOptions_.create_if_missing = true;
        // open DB
        Status s = DB::Open(
            rockesDBOptions_,
            ConfigManager::getInstance()->dbtmRocksDbPath(),
            &rocksDb_);
        if (!s.ok()) LOG(ERROR) << "open rocksDB error";
    }

    // 新建日志分发器
    logPlayer_ = new LogPlayer();
    logPlayer_->init(loop);

    // TODO: 待定增加线程数量
    pool_ = new ThreadPool(1);
}

void grit::Dbtl::solve(
    const muduo::net::TcpConnectionPtr &conn,
    const flat::DbtlMsg *dbtl)
{
    auto cmd = dbtl->cmd();

    switch (cmd) {
    case kLog:
        if (ConfigManager::getInstance()->dbtlUseRocksDb())
            writeToDiskByRocksDb(dbtl->txid(), dbtl->data(), conn);
        else
            writeToDisk(dbtl->txid(), dbtl->data(), conn);
        break;
    case kLsn:
        retResult(kLsn, dbtl->txid(), applyLsn_++, conn);
    default:
        LOG(ERROR) << "receive error cmd";
    }

    // TODO: 要不要做成分布式备份的形式
}

void grit::Dbtl::writeToDisk(
    int txid,
    const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *data,
    const TcpConnectionPtr &conn)
{
    int fd =
        open(to_string(txid).c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) LOG(ERROR) << "open log file error";

    // 以 id#属性:值 的形式进行存储
    int size = data->size();
    for (int i = 0; i < size; i++) {
        auto log = data->Get(i);

        string val;
        val += log->key()->str();
        val += '#';
        val += log->attribute()->str();
        val += ':';
        val += log->value()->str();
        val += '@';

        write(fd, val.c_str(), val.size());
        auto loginfo = new struct LogInfo(
            log->key()->str(), log->attribute()->str(), log->value()->str());

        logTable_[txid].emplace_back(loginfo);
    }

    close(fd);

    // TODO: 将日志发送给LogPlayer进行日志的分发
    pool_->enqueue(bind(&LogPlayer::playLog, logPlayer_, txid));

    retResult(kTranSuccess, txid, -1, conn);
}

void grit::Dbtl::writeToDiskByRocksDb(
    int txid,
    const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *data,
    const TcpConnectionPtr &conn)
{
    // 以 id#属性:值@ 的形式定义一条数据进行落盘
    WriteBatch batch;

    int size = data->size();
    for (int i = 0; i < size; i++) {
        auto log = data->Get(i);

        string val;
        val += log->attribute()->str();
        val += ':';
        val += log->value()->str();
        val += '@';

        batch.Put(log->key()->str(), val);
    }
    Status s = rocksDb_->Write(WriteOptions(), &batch);
    if (!s.ok()) LOG(ERROR) << "insert into rocksDB error";

    retResult(kTranSuccess, txid, -1, conn);
}

void Dbtl::retResult(int cmd, int txid, int lsn, const TcpConnectionPtr &conn)
{
    // 返回给上层结果
    flatbuffers::FlatBufferBuilder builder;
    flatbuffers::Offset<flat::DbtmMsg> dbtm;
    if (lsn == -1)
        dbtm = CreateDbtmMsg(builder, cmd, txid);
    else
        dbtm = CreateDbtmMsg(builder, cmd, txid, lsn);
    builder.Finish(dbtm);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    conn->send(ptr, size);
}