#pragma once

#include "group.hpp"
#include <string>
#include <vector>

class GroupModel
{
public:
    bool createGroup(Group &group);
    bool addGroup(int userId, int groupId, const std::string &role);
    std::vector<Group> queryGroups(int userId);                // Check the group info that the user has joined
    std::vector<int> queryGroupUsers(int groupId, int userId); // Check all users in the group except userId
};