#pragma once
// g++ mydb.cc -o mydb -std=c++11 -lmysqlclient
#ifndef __MYDB_H__
#    define __MYDB_H__

#    include <string>
#    include <mysql/mysql.h>

namespace grit {

class MyDb
{
  public:
    MyDb();
    ~MyDb();
    bool initDB(
        std::string host,
        std::string user,
        std::string pwd,
        std::string db_name,
        int port);
    bool exeSQL(std::string sql);

  private:
    MYSQL *mysql;
    MYSQL_RES *result;
    MYSQL_ROW row;

    // 打印完整的结果集，没调用
    // void getResult();
};

} // namespace grit

#endif

// 测试代码
// int main()
// {
//     MyDb db;

//     std::string host = "127.0.0.1";
//     std::string user = "root";
//     std::string passwd = "uestc8020";
//     std::string dbName = "test";
//     int port = 3306;

//     //连接数据库
//     bool conn = db.initDB(host, user, passwd, dbName, port);

//     if (!conn) { cout << "connect fails\n"; }

//     cout << "ok" << endl;

//     //将所有用户信息读出，并输出。
//     // std::string sql = "SELECT * from test;";
//     // db.exeSQL(sql);

//     return 0;
// }