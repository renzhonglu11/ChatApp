#pragma once

#include <string>
#include <vector>

class OffLineMsgModel
{
public:
    void insert(int userId, const std::string &msg);
    void remove(int userId);

    std::vector<std::string> query(int userId);

private:
};