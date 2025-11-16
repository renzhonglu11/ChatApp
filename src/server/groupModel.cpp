#include "groupModel.hpp"
#include "db.hpp"

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s');",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(conn));
            return true;
        }
    }
    return false;
}

// Add user to group
void GroupModel::addGroup(int userId, int groupId, const std::string &role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s');",
            userId, groupId, role.c_str());

    MySQL mysql;
    auto conn = mysql.getConnection();

    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// Query the groups that the user has joined
std::vector<Group> GroupModel::queryGroups(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid = %d;", userId);

    std::vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    // Query the users of each group
    for (Group &group : groupVec)
    {
        sprintf(sql, "select a.id, a.name, a.state, b.role from user a inner join groupuser b on b.userid = a.id where b.groupid = %d;", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupVec;
}

// Query all user IDs of a group
std::vector<int> GroupModel::queryGroupUsers(int groupId, int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d;", groupId, userId);

    std::vector<int> userIdVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                userIdVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return userIdVec;
}