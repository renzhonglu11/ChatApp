#pragma once
#include <string>

using namespace std;

// ORM user class
class User
{

public:
    User(int id = -1, string name = "", string password = "", string state = "offline")
    {
        this->_id = id;
        this->_name = name;
        this->_password = password;
    }

    int getId() const { return _id; }
    void setId(int id) { this->_id = id; }

    string getName() const { return this->_name; }
    void setName(const string &name) { this->_name = name; }

    string getPassword() const { return this->_password; }
    void setPassword(const string &password) { this->_password = password; }

private:
    int _id;          // User ID
    string _name;     // Username
    string _password; // User password
};