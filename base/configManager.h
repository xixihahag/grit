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
    std::string address() { return address_; }
    int port() { return port_; }
    int threads() { return threads_; }
    std::string transactionsDir() { return transactionsDir_; }

  private:
    // basis
    std::string logDir_;

    // gtm
    std::string address_;
    int port_;
    int threads_;
    std::string transactionsDir_;
};

} // namespace grit