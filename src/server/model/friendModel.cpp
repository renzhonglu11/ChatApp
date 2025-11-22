#include "friendModel.hpp"
#include "db.hpp"

void FriendModel::addFriend(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userid, friendid) values(%d, %d);",
            userId, friendId);

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::queryFriends(int userId)
{
    std::vector<User> friendList;
    char sql[1024] = {0};
    sprintf(sql, "select u.id, u.name, u.state from user u inner join friend f on u.id = f.friendid where f.userid = %d;",
            userId);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friendList.push_back(user);
            }
            mysql_free_result(res);
            return friendList;
        }
    }
    return friendList;
}