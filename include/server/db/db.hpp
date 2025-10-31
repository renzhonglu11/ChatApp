#pragma once

#include <mysql/mysql.h>
#include <string>

using namespace std;

static string server = "127.0.0.1";
static string user = "root";
static string password = "1995106x";
static string database = "chat";

class MySQL
{
public:
    MySQL();
    ~MySQL();

    // Connect to database
    bool connect();
    // Update operation
    bool update(string sql);

    // Query operation
    MYSQL_RES *query(string sql);

    MYSQL *getConnection();

private:
    MYSQL *_conn;
};