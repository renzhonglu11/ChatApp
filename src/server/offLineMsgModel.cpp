#include "offLineMsgModel.hpp"
#include "db.hpp"

void OffLineMsgModel::insert(int userId, const std::string &msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s');",
            userId, msg.c_str());

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void OffLineMsgModel::remove(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d;",
            userId);

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<std::string> OffLineMsgModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d;", userId);

    MySQL mysql;
    vector<std::string> msgs;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            while (row != nullptr)
            {
                msgs.push_back(row[0]);
                row = mysql_fetch_row(res);
            }
            mysql_free_result(res);
        }
    }
    return msgs;
}
