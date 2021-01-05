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

    // dbtl
    SetOrDefaultS(dbtlAddress_, pt.get<string>({"dbtl.address"}));
    SetOrDefaultI(dbtlPort_, pt.get<int>("dbtl.port"));

    // dbtm
    SetOrDefaultI(dbtmThreadNum_, pt.get<int>("dbtm.threadnum"));
    SetOrDefaultS(rocksDbPath_, pt.get<string>({"dbtm.rocksdbpath"}));

    // dbservice
    SetOrDefaultS(dbserviceAddress_, pt.get<string>({"dbservice.address"}));
    SetOrDefaultI(dbservicePort_, pt.get<int>("dbservice.port"));

    // db
    SetOrDefaultS(dbAddress_, pt.get<string>({"db.dbaddress"}));
    SetOrDefaultI(dbPort_, pt.get<int>("db.dbport"));
    SetOrDefaultS(dbName_, pt.get<string>({"db.dbname"}));
    SetOrDefaultS(dbPasswd_, pt.get<string>({"db.dbpasswd"}));
}

} // namespace grit