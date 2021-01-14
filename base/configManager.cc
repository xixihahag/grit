#include "configManager.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#define SetOrDefaultS(variable, value)                                         \
    if (value != "") variable = value;
#define SetOrDefaultI(variable, value)                                         \
    if (value != 0) variable = value;
#define SetOrDefaultB(variable, value) variable = value;

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
    SetOrDefaultS(gtmListenAddress_, pt.get<string>({"gtm.listenaddress"}));
    SetOrDefaultS(gtmAddress_, pt.get<string>({"gtm.address"}));
    SetOrDefaultI(gtmPort_, pt.get<int>("gtm.port"));
    SetOrDefaultI(gtmThreads_, pt.get<int>("gtm.threads"));
    SetOrDefaultS(gtmTransactionsDir_, pt.get<string>("gtm.transactionsDir"));

    // dbservice
    SetOrDefaultS(
        dbsListenAddress_, pt.get<string>({"dbservice.listenaddress"}));
    SetOrDefaultS(dbsAddress_, pt.get<string>({"dbservice.address"}));
    SetOrDefaultI(dbsPort_, pt.get<int>("dbservice.port"));
    SetOrDefaultI(dbsThreads_, pt.get<int>("dbservice.threads"));

    // db
    SetOrDefaultS(dbAddress_, pt.get<string>({"db.dbaddress"}));
    SetOrDefaultI(dbPort_, pt.get<int>("db.dbport"));
    SetOrDefaultS(dbName_, pt.get<string>({"db.dbname"}));
    SetOrDefaultS(dbPasswd_, pt.get<string>({"db.dbpasswd"}));

    // dbtm
    SetOrDefaultI(dbtmThreadNum_, pt.get<int>("dbtm.threadnum"));
    SetOrDefaultS(dbtmRocksDbPath_, pt.get<string>({"dbtm.rocksdbpath"}));

    // dbtl
    SetOrDefaultS(dbtlListenAddress_, pt.get<string>({"dbtl.listenaddress"}));
    SetOrDefaultS(dbtlAddress_, pt.get<string>({"dbtl.address"}));
    SetOrDefaultI(dbtlPort_, pt.get<int>("dbtl.port"));
    SetOrDefaultB(dbtlUseRocksDb_, pt.get<bool>("dbtl.userocksdb"));

    // es
    SetOrDefaultS(esListenAddress_, pt.get<string>({"es.listenaddress"}));
    SetOrDefaultI(esPort_, pt.get<int>("es.port"));
}

} // namespace grit