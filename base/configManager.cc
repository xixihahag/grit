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

void ConfigManager::init(const char *configDir)
{
    ptree pt;
    read_ini(configDir, pt);

    // basis
    SetOrDefaultS(logDir_, pt.get<string>("basis.logDir"));

    // gtm
    SetOrDefaultS(gtmAddress_, pt.get<string>({"gtm.address"}));
    SetOrDefaultI(gtmPort_, pt.get<int>("gtm.port"));
    SetOrDefaultI(threads_, pt.get<int>("gtm.threads"));
    SetOrDefaultS(transactionsDir_, pt.get<string>("gtm.transactionsDir"));

    // dbtm
    SetOrDefaultS(dbtlAddress_, pt.get<string>({"gtm.address"}));
    SetOrDefaultI(dbtlPort_, pt.get<int>("gtm.port"));
}

} // namespace grit