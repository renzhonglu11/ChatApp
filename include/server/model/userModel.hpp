#pragma once

#include "user.hpp"

// User data operation class
class UserModel
{
public:
    bool insert(User &user);
    User query(int id);
    User queryByName(const string &name);
    bool updateState(User &user);
    void resetState();
};
