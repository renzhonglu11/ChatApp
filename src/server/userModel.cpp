#include "userModel.hpp"
#include "db.hpp"

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
