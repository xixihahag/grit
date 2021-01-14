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
using namespace ROCKSDB_NAMESPACE;

grit::Dbtl::Dbtl()
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
}

void grit::Dbtl::solve(
    const muduo::net::TcpConnectionPtr &conn,
    const flat::DbtlMsg *dbtl)
{
    auto cmd = dbtl->cmd();

    switch (cmd) {
    case kLog:
        if (ConfigManager::getInstance()->dbtlUseRocksDb())
            writeToDiskByRocksDb(dbtl->txid(), dbtl->data());
        else
            writeToDisk(dbtl->txid(), dbtl->data());
    }

    // 返回给对面 ack
    flatbuffers::FlatBufferBuilder builder;
    auto dbtm = CreateDbtl(builder, kLogAck, dbtl->txid());
    builder.Finish(dbtm);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    conn->send(ptr, size);
}

void grit::Dbtl::writeToDisk(
    int txid,
    const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *data)
{
    int fd =
        open(to_string(txid).c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) LOG(ERROR) << "open log file error";

    // 以 标签#属性:值 的形式进行存储
    int size = data->size();
    for (int i = 0; i < size; i++) {
        auto log = data->Get(i);

        string val;
        val += log->label()->str();
        val += '#';
        val += log->attribute()->str();
        val += ':';
        val += log->value()->str();
        val += '@';

        write(fd, val.c_str(), val.size());
    }

    close(fd);
}

void grit::Dbtl::writeToDiskByRocksDb(
    int txid,
    const flatbuffers::Vector<flatbuffers::Offset<flat::Data> > *data)
{
    // 以 标签#属性:值@ 的形式定义一条数据进行落盘
    WriteBatch batch;

    int size = data->size();
    for (int i = 0; i < size; i++) {
        auto log = data->Get(i);

        string val;
        val += log->attribute()->str();
        val += ':';
        val += log->value()->str();
        val += '@';

        batch.Put(log->label()->str(), val);
    }
    Status s = rocksDb_->Write(WriteOptions(), &batch);
    if (!s.ok()) LOG(ERROR) << "insert into rocksDB error";
}