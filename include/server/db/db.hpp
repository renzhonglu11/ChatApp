#pragma once

#include <mysql/mysql.h>
#include <string>

using namespace std;

class MySQL {
public:
    MySQL();
    ~MySQL();

    // Connect to database
    bool connect();
    // Update operation
    bool update(string sql);

    // Query operation
    MYSQL_RES* query(string sql);

    MYSQL* getConnection();

private:
    MYSQL* _conn;
    string _server;
    string _user;
    string _password;
    string _database;
    unsigned int _port;
};