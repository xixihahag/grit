// g++ mydb.cc -o mydb -std=c++11 -lmysqlclient
#pragma once
#include <string>
#include <mysql/mysql.h>
#include <glog/logging.h>

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
    void getResult();
};

MyDb::MyDb()
{
    mysql = mysql_init(NULL);

    if (!mysql) {
        LOG(ERROR) << "Error:" << mysql_error(mysql);
        exit(1);
    }
}

MyDb::~MyDb()
{
    if (mysql) { mysql_close(mysql); }
}

bool MyDb::initDB(
    std::string host,
    std::string user,
    std::string passwd,
    std::string db_name,
    int port = 3306)
{
    mysql = mysql_real_connect(
        mysql,
        host.c_str(),
        user.c_str(),
        passwd.c_str(),
        db_name.c_str(),
        port,
        NULL,
        0);
    if (!mysql) {
        LOG(ERROR) << "Error: " << mysql_error(mysql);
        exit(1);
    }

    return true;
}

bool MyDb::exeSQL(std::string sql)
{
    // mysql_query()执行成功返回0,执行失败返回非0值。
    if (mysql_query(mysql, sql.c_str())) {
        LOG(ERROR) << "Query Error: " << mysql_error(mysql);
        return false;
    }

    // getResult();

    return true;
}

void MyDb::getResult()
{
    result = mysql_store_result(mysql);

    if (result) {
        // 获取结果集中总共的字段数，即列数
        // 输出完整结果集，没啥用
        int num_fields = mysql_num_fields(result);
        unsigned long long num_rows = mysql_num_rows(result);

        for (unsigned long long i = 0; i < num_rows; i++) {
            row = mysql_fetch_row(result);
            if (!row) { break; }
            std::string str;
            for (int j = 0; j < num_fields; j++) {
                str += row[j];
                str += "\t\t";
            }
            LOG(INFO) << str;
        }
    } else {
        //代表执行的是update,insert,delete类的非查询语句
        if (mysql_field_count(mysql) == 0) {
            // 返回update,insert,delete影响的行数
            unsigned long long num_rows = mysql_affected_rows(mysql);

            // return num_rows;
            return;
        } else {
            LOG(ERROR) << "Get result error: " << mysql_error(mysql);
            // return false;
            return;
        }
    }

    // return true;
    return;
}

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