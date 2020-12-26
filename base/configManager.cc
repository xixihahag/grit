#include "configManager.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#define SetOrDefaultS(variable, value)                                         \
    if (value != "") variable = value;
#define SetOrDefaultI(variable, value)                                         \
    if (value != 0) variable = value;

using namespace std;
using namespace boost::property_tree;

namespace grit {
ConfigManager::ConfigManager() {}

ConfigManager::ConfigManager(string &configDir)
{
    ptree pt;
    read_ini(configDir, pt);

    SetOrDefaultS(logDir_, pt.get<string>("bases.logDir"));
}

} // namespace grit