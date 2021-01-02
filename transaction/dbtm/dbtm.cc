#include "dbtm.h"
#include "headerCmd.h"
// #include "dbservice/dbservice.h"

using namespace grit;
using namespace std;
using namespace flat;

void Dbtm::judgeLocalConflict(struct transaction *trans)
{
    auto lsn = trans->lsn;

    bool haveConflict = false;
    for (auto it = rcheck.begin(); it != rcheck.end() && !haveConflict;) {
        if (it->first > lsn) {
            for (auto rs : trans->readSet)
                if (it->second.find(rs->key) != it->second.end()) {
                    haveConflict = true;
                    trcheck.clear();
                    break;
                } else
                    trcheck[lsn].emplace(rs->key);

            it++;
        } else
            it = rcheck.erase(it);
    }

    for (auto it = wcheck.begin(); it != wcheck.end() && !haveConflict;) {
        if (it->first > lsn) {
            for (auto ws : trans->writeSet)
                if (it->second.find(ws->key) != it->second.end()) {
                    haveConflict = true;
                    twcheck.clear();
                    break;
                } else
                    twcheck[lsn].emplace(ws->key);

            it++;
        } else
            it = wcheck.erase(it);
    }

    if (!haveConflict) {}
}

void Dbtm::getLsnAndGlobalConflict(int txid) {}

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

    for (auto read : trans->readSet)
        rcheck[lsn].emplace(read->key);

    for (auto write : trans->writeSet)
        wcheck[lsn].emplace(write->key);

    sendLog();
}