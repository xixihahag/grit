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
    std::string gtmAddress() { return gtmAddress_; }
    int gtmPort() { return gtmPort_; }
    int threads() { return threads_; }
    std::string transactionsDir() { return transactionsDir_; }

    // dbtl
    std::string dbtlAddress() { return dbtlAddress_; }
    int dbtlPort() { return dbtlPort_; }

    // dbtm
    int dbtmThreadNum() { return dbtmThreadNum_; }
    std::string rocksDbPath() { return rocksDbPath_; }

    // dbservice
    std::string dbserviceAddress() { return dbserviceAddress_; }
    int dbservicePort() { return dbservicePort_; }
    std::string dbaddress() { return dbAddress_; }
    int abPort() { return dbPort_; }
    std::string dbName() { return dbName_; }
    std::string dbPasswd() { return dbPasswd_; }

  private:
    // basis
    std::string logDir_;

    // gtm
    std::string gtmAddress_;
    int gtmPort_;
    int threads_;
    std::string transactionsDir_;

    // dbservice
    std::string dbserviceAddress_;
    int dbservicePort_;

    // db
    std::string dbAddress_;
    int dbPort_;
    std::string dbName_;
    std::string dbPasswd_;

    // dbtl
    std::string dbtlAddress_;
    int dbtlPort_;

    // dbtm
    int dbtmThreadNum_;
    std::string rocksDbPath_;
};

} // namespace grit