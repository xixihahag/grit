#include "dbservice.h"
#include "configManager.h"
#include "headerCmd.h"

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

DbService::DbService(muduo::net::EventLoop *loop)
{
    dbtm_ = new Dbtm(this);
    dbtm_->init(loop);

    // 开启线程池
    threadPool_ = new ThreadPool(ConfigManager::getInstance()->dbtmThreadNum());
}

void DbService::getReadWriteSet(
    const TcpConnectionPtr &conn,
    const DbServiceMsg *data)
{
    transaction *tran;
    if (txidTrans_.find(data->txid()) == txidTrans_.end()) {
        tran = new transaction();
        txidTrans_[data->txid()] = tran;
        tran->txid = data->txid();
    } else
        tran = txidTrans_[data->txid()];

    table_[tran->txid] = conn;

    // 讲道理这个参数应该是恒为1
    int rsize = data->readSet()->size();

    for (int i = 0; i < rsize; i++) {
        auto rset = data->readSet()->Get(i);

        Data *rdata = new Data(
            rset->key()->str(),
            "",
            rset->attribute()->str(),
            rset->value()->str());
        tran->readSet.emplace_back(rdata);
    }

    int wsize = data->writeSet()->size();
    for (int i = 0; i < wsize; i++) {
        auto wset = data->writeSet()->Get(i);

        Data *wdata = new Data(
            wset->key()->str(),
            "",
            wset->attribute()->str(),
            wset->value()->str());

        tran->writeSet.emplace_back(wdata);
    }
}

void DbService::retResult(int status, int txid)
{
    flatbuffers::FlatBufferBuilder builder;

    auto es = CreateESMsg(builder, status, txid);
    auto msg = CreateRootMsg(builder, Msg_ESMsg, es.Union());
    builder.Finish(msg);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    table_[txid]->send(ptr, size);

    // 把本地映射去掉，回收空间
    auto tran = dbtm_->table_[txid];
    dbtm_->table_.erase(txid);

    delete tran;
}