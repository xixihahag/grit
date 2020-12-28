#include <iostream>
#include <string>
#include <fstream>
#include <glog/logging.h>
#include "gtm.h"
#include "flatbuffers/flatbuffers.h"
#include "net_generated.h"
#include "configManager.h"

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

void GTM::getTxid(const TcpConnectionPtr &conn)
{
    flatbuffers::FlatBufferBuilder builder;

    auto gtmAck = CreateGtmAck(builder, txid_.load());
    builder.Finish(gtmAck);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    conn->send(ptr, size);
}

void GTM::judgeConflict()
{
    // TODO: 待定
}

void GTM::init()
{
    string dir = ConfigManager::getInstance()->transactionsDir();

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

                transInfo_[type].emplace(tmp);
                st = ed + 1;
            }
        }
    }
}