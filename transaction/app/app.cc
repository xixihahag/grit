#include "app.h"
#include "headerCmd.h"
#include "configManager.h"
#include "muduo/net/TcpClient.h"
#include <glog/logging.h>

using namespace std;
using namespace grit;
using namespace flat;
using namespace muduo::net;

App ::App(EventLoop *loop)
{
    loop_ = loop;

    const char *ip = ConfigManager::getInstance()->gtmAddress().c_str();
    uint16_t port =
        static_cast<uint16_t>(ConfigManager::getInstance()->gtmPort());
    InetAddress servAddr(ip, port);
    TcpClient *gtmClient_ = new TcpClient(loop, servAddr, "gtm");
    gtmClient_->connect();
    gtmClient_->setConnectionCallback(
        bind(&onGtmConnection, this, std::placeholders::_1));
    gtmClient_->setMessageCallback(bind(
        &onMessage,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3));
}

void App::onMessage(
    const TcpConnectionPtr &conn,
    Buffer *buf,
    muduo ::Timestamp)
{
    auto msg = GetRootMsg((uint8_t *) buf->retrieveAllAsString().c_str());
    auto data = static_cast<const AppMsg *>(msg->any());
    auto cmd = data->cmd();

    switch (cmd) {
    case kTxid:
        // 解析txid存储，解析es列表进行建连
        txid_ = data->txid();
        connect2ES(data->list());
        break;
    default:
        LOG(ERROR) << "receive error cmd";
    }
}

void App::onGtmConnection(const muduo::net::TcpConnectionPtr &conn)
{
    gtmConn_ = conn;
}

void App::connect2ES(
    const flatbuffers::Vector<flatbuffers::Offset<flat::ipAndPort> > *data)
{
    int size = data->size();

    for (int i = 0; i < size; i++) {
        auto ipandport = data->Get(i);

        string ip = ipandport->ip()->str();
        int port = ipandport->port();

        InetAddress servAddr(ip, port);
        TcpClient *esClient_ = new TcpClient(loop_, servAddr, "dbs");
        esClient_->connect();

        esClient_->setConnectionCallback(
            bind(&onESConnection, this, std::placeholders::_1));
    }
}

void App::onESConnection(const muduo::net::TcpConnectionPtr &conn)
{
    connList_.emplace_back(conn);
}

/*
    add node key attr val
    add edge key attr val

    change node key attr val
    change edge key attr val

    delete node key
    delete edge key

    search node key
    search delete key
*/
void App::add(string str)
{
    flatbuffers::FlatBufferBuilder builder;

    // 先做一个解析，然后把结果通知给es
    int pos1 = 0, pos2 = 0, len = str.size();
    while (pos2 < len && str[pos2] != ' ')
        pos2++;

    // add change delete search
    int cmd;
    string type = str.substr(pos1, pos2 - pos1);
    pos1 = ++pos2;
    if (strcmp(type.c_str(), "add") == 0)
        cmd = kAdd;
    else if (strcmp(type.c_str(), "delete") == 0)
        cmd = kDelete;
    else if (strcmp(type.c_str(), "Change") == 0)
        cmd = kChange;
    else if (strcmp(type.c_str(), "Search") == 0)
        cmd = kSearch;
    else
        LOG(ERROR) << "order error";

    // type : node or edge
    while (pos2 < len && str[pos2] != ' ')
        pos2++;
    int typ = stoi(str.substr(pos1, pos2 - pos1));
    pos1 = ++pos2;

    // key
    while (pos2 < len && str[pos2] != ' ')
        pos2++;
    auto key = builder.CreateString(str.substr(pos1, pos2 - pos1));
    pos1 = ++pos2;

    char *ptr = nullptr;
    uint64_t size = 0;

    if (cmd == kAdd || cmd == kChange) {
        // attr
        while (pos2 < len && str[pos2] != ' ')
            pos2++;
        auto attr = builder.CreateString(str.substr(pos1, pos2 - pos1));
        pos1 = ++pos2;

        // val
        while (pos2 < len && str[pos2] != ' ')
            pos2++;
        auto val = builder.CreateString(str.substr(pos1, pos2 - pos1));

        auto es = CreateESMsg(builder, cmd, txid_, false, typ, key, attr, val);
        builder.Finish(es);

        ptr = (char *) builder.GetBufferPointer();
        size = builder.GetSize();
    } else {
        auto es = CreateESMsg(builder, cmd, txid_, false, typ, key);
        builder.Finish(es);

        ptr = (char *) builder.GetBufferPointer();
        size = builder.GetSize();
    }
    // FIXME:
    // 只有一个数据库的情况下可以这么干，多个数据库需要有一个方法来识别发给哪个连接
    connList_.front()->send(ptr, size);
}

void App::commit()
{
    flatbuffers::FlatBufferBuilder builder;
    auto es = CreateESMsg(
        builder, kCommit, txid_, connList_.size() == 1 ? false : true);
    builder.Finish(es);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    // 给每一个es发送提交命令
    for (auto it = connList_.begin(); it != connList_.end(); it++)
        it->get()->send(ptr, size);
}

void App::startTran(std::string type)
{
    // 向gtm要txid和dbs列表
    flatbuffers::FlatBufferBuilder builder;
    auto trantype = builder.CreateString(type);
    auto gtm = CreateGtmMsg(builder, kGetTxid, trantype);
    builder.Finish(gtm);

    char *ptr = (char *) builder.GetBufferPointer();
    uint64_t size = builder.GetSize();

    gtmConn_->send(ptr, size);
}

void App::showResult(int status, int txid)
{
    if (status == kTranSuccess)
        LOG(INFO) << "[" << txid << "]transaction executed successfully";
    else
        LOG(INFO) << "[" << txid << "]transaction execution failed";
}