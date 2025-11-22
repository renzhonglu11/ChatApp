#pragma once

#include "user.hpp"

class GroupUser : public User
{
public:
    void setRole(const std::string &role)
    {
        this->_role = role;
    }

    std::string getRole() const
    {
        return this->_role;
    }

private:
    std::string _role;
};