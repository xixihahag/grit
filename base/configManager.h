#pragma once

#include <string>
#include "singleton.h"

namespace grit {

class ConfigManager : public Singleton<ConfigManager>
{
    friend class Singleton<ConfigManager>;

  public:
    void init(const char *);
    // base
    std::string logDir() { return logDir_; }

    // gtm
    std::string gtmListenAddress() { return gtmListenAddress_; }
    std::string gtmAddress() { return gtmAddress_; }
    int gtmPort() { return gtmPort_; }
    int gtmThreads() { return gtmThreads_; }
    std::string gtmTransactionsDir() { return gtmTransactionsDir_; }

    // dbservice
    std::string dbsListenAddress() { return dbsListenAddress_; }
    std::string dbsAddress() { return dbsAddress_; }
    int dbsPort() { return dbsPort_; }
    int dbsThreads() { return dbsThreads_; }

    // db
    std::string dbaddress() { return dbAddress_; }
    int abPort() { return dbPort_; }
    std::string dbName() { return dbName_; }
    std::string dbPasswd() { return dbPasswd_; }

    // dbtm
    int dbtmThreadNum() { return dbtmThreadNum_; }
    std::string dbtmRocksDbPath() { return dbtmRocksDbPath_; }
    std::string dbtmUser() { return dbtmUser_; }
    std::string dbtmPasswd() { return dbtmPasswd_; }
    std::string dbtmDbName() { return dbtmDbName_; }
    int dbtmDbPort() { return dbtmDbPort_; }
    std::string dbtmLogPath() { return dbtmLogPath_; }

    // dbtl
    std::string dbtlListenAddress() { return dbtlListenAddress_; }
    std::string dbtlAddress() { return dbtlAddress_; }
    int dbtlPort() { return dbtlPort_; }
    bool dbtlUseRocksDb() { return dbtlUseRocksDb_; }

    // es
    std::string esListenAddress() { return esListenAddress_; }
    int esPort() { return esPort_; }

  private:
    // basis
    std::string logDir_;

    // gtm
    std::string gtmListenAddress_;
    std::string gtmAddress_;
    int gtmPort_;
    int gtmThreads_;
    std::string gtmTransactionsDir_;

    // dbservice
    std::string dbsListenAddress_;
    std::string dbsAddress_;
    int dbsPort_;
    int dbsThreads_;

    // db
    std::string dbAddress_;
    int dbPort_;
    std::string dbName_;
    std::string dbPasswd_;

    // dbtm
    int dbtmThreadNum_;
    std::string dbtmRocksDbPath_;
    std::string dbtmUser_;
    std::string dbtmPasswd_;
    std::string dbtmDbName_;
    int dbtmDbPort_;
    std::string dbtmLogPath_;

    // dbtl
    std::string dbtlListenAddress_;
    std::string dbtlAddress_;
    int dbtlPort_;
    bool dbtlUseRocksDb_;

    // es
    std::string esListenAddress_;
    int esPort_;
};

} // namespace grit