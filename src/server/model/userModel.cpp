#include "userModel.hpp"
#include "db.hpp"

void UserModel::resetState()
{
    // Set all users' state to 'offline' in the database
    char sql[1024] = "update user set state = 'offline' where state = 'online';";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

bool UserModel::insert(User &user)
{

    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password) values('%s', '%s');",
            user.getName().c_str(), user.getPassword().c_str());

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // Get the inserted user ID

            user.setId(mysql_insert_id(conn)); // Returns the value generated for an AUTO_INCREMENT column by the previous INSERT or UPDATE statement.
            return true;
        }
    }

    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d;", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User(); // return empty user object if not found
}

User UserModel::queryByName(const string &name)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where name = '%s';", name.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}

bool UserModel::updateState(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d;",
            user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connect())
    {
        return mysql.update(sql);
    }
    return false;
}
