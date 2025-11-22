#pragma once

#include <vector>
#include <string>
#include "user.hpp"

class FriendModel
{
public:
    void addFriend(int userId, int friendId);

    // Return the friend list of a user. The list contains user's name and state, etc.
    std::vector<User> queryFriends(int userId);
};