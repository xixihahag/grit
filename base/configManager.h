#pragma once

#include <string>
#include "singleton.h"

namespace grit {

class ConfigManager : public Singleton<ConfigManager>
{
  public:
    ConfigManager();
    ConfigManager(std::string &);

    std::string logDir() { return logDir_; }

  private:
    std::string logDir_;
};

} // namespace grit