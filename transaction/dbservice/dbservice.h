/*
 * 相当于数据库的api，可以直接跟数据库联系
 */

#pragma once

#include <string>
#include <list>
#include <unordered_set>
#include "net_generated.h"

namespace grit {

class DbService
{
  public:
    DbService();

    DbService(
        std::string ip,
        int port,
        std::string database,
        std::string passwd)
        : ip_(ip)
        , port_(port)
        , database_(database)
        , passwd_(passwd)
    {}

    // 连接远端的数据库
    void connectToDatabase();

    // 从传输过来的数据中解析读写集
    void getReadWriteSet(const flat::DbService *);

  private:
    struct writeData
    {
        writeData(std::string k, std::string rec, std::string val)
            : key(k)
            , record(rec)
            , value(val)
        {}

        std::string key;
        std::string record;
        std::string value;
    };

    struct readData
    {
        readData(std::string k)
            : key(k)
        {}

        std::string key;
    };

    struct transaction
    {
        std::list<readData> readSet;
        std::list<writeData> writeSet;
        std::string lsn;
    };

    // // FIXME: 看看这个数据结构应该放在哪
    // struct Dbtm
    // {
    //     Dbtm(int t)
    //         : type(t)
    //     {}
    //     int type;
    //     int txid;
    //     bool isConflict;
    //     int lsn;
    // };

    // 用于数据库的连接,暂时无用
    std::string ip_;
    int port_;
    std::string database_;
    std::string passwd_;

  public:
    std::list<transaction *> transaction_;

    // 用于快速进行数据冲突验证
    std::unordered_set<string> rcheck, wcheck;
};

} // namespace grit