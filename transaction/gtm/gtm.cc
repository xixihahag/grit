#include <iostream>
#include <string>
#include <fstream>
#include <glog/logging.h>
#include <vector>
#include "gtm.h"
#include "flatbuffers/flatbuffers.h"
#include "configManager.h"
#include "headerCmd.h"

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

void GTM::getTxid(const TcpConnectionPtr &conn, string transType)
{
    flatbuffers::FlatBufferBuilder builder;
    auto list = transInfo_[transType];

    if (list.size() > 1)
        table_[txid_.load()] = new struct txidInfo(list.size());

    vector<flatbuffers::Offset<ipAndPort> > ipAndPortVec;
    for (auto it = list.begin(); it != list.end(); it++) {
        auto ip = builder.CreateString((*it)->ip_);
        auto data = CreateipAndPort(builder, ip, (*it)->port_);
        ipAndPortVec.push_back(data);
    }
    auto ipAndPort = builder.CreateVector(ipAndPortVec);

    auto app = CreateAppMsg(builder, kTxid, txid_.load(), ipAndPort);
    auto msg = CreateRootMsg(builder, Msg_AppMsg, app.Union());
    builder.Finish(msg);

    txid_++;

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    conn->send(ptr, size);
}

void GTM::judgeConflict(
    const muduo::net::TcpConnectionPtr &conn,
    const GtmMsg *data)
{
    int txid = data->txid();
    auto info = table_[txid];
    info->connList_.emplace_back(conn);
    info->isConflict |= data->isLocalConflict();

    if (--info->serverNum == 0) {
        for (auto conn : info->connList_) {
            flatbuffers::FlatBufferBuilder builder;
            auto dbtm =
                CreateDbtmMsg(builder, kJudgeConflit, txid, info->isConflict);
            auto msg = CreateRootMsg(builder, Msg_DbtmMsg, dbtm.Union());
            builder.Finish(msg);

            char *ptr = (char *) builder.GetBufferPointer();
            uint64_t size = builder.GetSize();

            conn->send(ptr, size);
        }

        table_.erase(txid);
        delete info;
    }
}

void GTM::init()
{
    string dir = ConfigManager::getInstance()->gtmTransactionsDir();

    // 读取文件信息
    ifstream fin(dir.c_str());

    string type, info;

    while (fin >> type) {
        fin >> info;
        size_t st = 0, ed = 0, pos = -1;
        for (; ed < info.size(); ed++) {
            if (info[ed] == ':')
                pos = ed;
            else if (info[ed] == ';' && ed > st) {
                IpandPort *tmp = new IpandPort(
                    info.substr(st, pos - st + 1),
                    stoi(info.substr(pos + 1, ed - pos - 1)));

                transInfo_[type].emplace_back(tmp);
                st = ed + 1;
            }
        }
    }
}