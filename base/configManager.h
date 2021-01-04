#pragma once

#include <string>
#include "singleton.h"

namespace grit {

class ConfigManager : public Singleton<ConfigManager>
{
    friend class Singleton<ConfigManager>;

  public:
    void init(const char *);
    std::string logDir() { return logDir_; }

    std::string gtmAddress() { return gtmAddress_; }
    int gtmPort() { return gtmPort_; }
    int threads() { return threads_; }
    std::string transactionsDir() { return transactionsDir_; }

    std::string dbtlAddress() { return dbtlAddress_; }
    int dbtlPort() { return dbtlPort_; }

    int dbtmThreadNum() { return dbtmThreadNum_; }

  private:
    // basis
    std::string logDir_;

    // gtm
    std::string gtmAddress_;
    int gtmPort_;
    int threads_;
    std::string transactionsDir_;

    // dbtl
    std::string dbtlAddress_;
    int dbtlPort_;

    // dbtm
    int dbtmThreadNum_;
};

} // namespace grit