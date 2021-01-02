#include "dbservice.h"

using namespace std;
using namespace grit;

void DbService::getReadWriteSet(const flat::DbService *data)
{
    transaction *tran = new transaction();

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

    transaction_.emplace_back(tran);
}