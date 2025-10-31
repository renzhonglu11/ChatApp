#include <db.hpp>
#include <muduo/base/Logging.h>

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

MYSQL *MySQL::getConnection()
{
    return _conn;
}

// Connect to database
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), database.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // Set character set to utf8
        mysql_set_character_set(_conn, "utf8");
        LOG_INFO << "MySQL Connect Success!";
        return true;
    }
    LOG_ERROR << "MySQL Connect Fail!";
    return false;
}

// Update operation
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << " - " << mysql_error(_conn);
        return false;
    }
    return true;
}

// Query operation
MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << " - " << mysql_error(_conn);
        return nullptr;
    }
    return mysql_store_result(_conn);
}