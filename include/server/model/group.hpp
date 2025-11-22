#pragma once

#include <string>
#include <vector>
#include "groupUser.hpp"

class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "") : _id(id), _name(name), _desc(desc) {}

    void setId(int id)
    {
        this->_id = id;
    }
    void setName(const std::string &name)
    {
        this->_name = name;
    }
    void setDesc(const std::string &desc)
    {
        this->_desc = desc;
    }

    int getId() const
    {
        return this->_id;
    }
    std::string getName() const
    {
        return this->_name;
    }
    std::string getDesc() const
    {
        return this->_desc;
    }

    const std::vector<GroupUser> &getUsers() const
    {
        return this->_userVec;
    }

    void addUser(const GroupUser &user)
    {
        this->_userVec.push_back(user);
    }

private:
    int _id;
    std::string _name;
    std::string _desc;
    std::vector<GroupUser> _userVec;
};