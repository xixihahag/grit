#include "dbservice.h"

using namespace std;
using namespace grit;

void DbService::getReadWriteSet(const flat::DbService *data)
{
    transaction *tran = new transaction();
    tran->txid = data->txid();

    int rsize = data->readSet()->size();
    for (int i = 0; i < rsize; i++) {
        auto rset = data->readSet()->Get(i);

        readData *rdata = new readData(rset->key()->str());
        tran->readSet.emplace_back(rdata);

        // rcheck.emplace(rdata->key);
    }

    int wsize = data->writeSet()->size();
    for (int i = 0; i < wsize; i++) {
        auto wset = data->writeSet()->Get(i);

        writeData *wdata = new writeData(
            wset->key()->str(), wset->record()->str(), wset->value()->str());

        tran->writeSet.emplace_back(wdata);

        // wcheck.emplace(wdata->key);
    }

    // TODO: 发送给DBTM处理 考虑用不用线程池 讲道理n方的复杂度，还是用起来比较好

    // transactions_.emplace_back(tran);
}