#include "dbservice.h"
#include "configManager.h"

using namespace std;
using namespace grit;
using namespace flat;

void DbService::test() {}

DbService::DbService(muduo::net::EventLoop *loop)
{
    dbtm_ = new Dbtm(this);
    dbtm_->init(loop);

    // 开启线程池
    threadPool_ = new ThreadPool(ConfigManager::getInstance()->dbtmThreadNum());
}

void DbService::getReadWriteSet(
    const muduo::net::TcpConnectionPtr &conn,
    const DbServiceMsg *data)
{
    transaction *tran = new transaction();
    tran->txid = data->txid();

    table_[tran->txid] = conn;

    int rsize = data->readSet()->size();

    for (int i = 0; i < rsize; i++) {
        auto rset = data->readSet()->Get(i);

        Data *rdata = new Data(
            rset->key()->str(),
            "",
            rset->attribute()->str(),
            rset->value()->str());
        tran->readSet.emplace_back(rdata);

        tran->trcheck.emplace(rdata->key);
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

        tran->twcheck.emplace(wdata->key);
    }

    tran->needGlobalConflct = data->needGlobalConflict();

    // 发送给DBTM处理 考虑用不用线程池 讲道理n方的复杂度，还是用起来比较好
    threadPool_->enqueue(bind(&Dbtm::judgeLocalConflict, dbtm_, tran));
}