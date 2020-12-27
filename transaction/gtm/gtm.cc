#include <iostream>
#include "gtm.h"
#include "flatbuffers/flatbuffers.h"
#include "net_generated.h"

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