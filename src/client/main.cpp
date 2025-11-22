#include "json.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

User g_currentUser;                        // Store the current logged-in user information
std::vector<User> g_currentUserFriendList; // Store the current user's friend list
std::vector<Group> g_currentUserGroupList; // Store the current user's group list

void showCurrentUserData();
void readTaskHandler(int clientfd);
string getCurrentTime();
void mainMenu();

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: ./ChatClient <IP> <Port>" << std::endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "Socket creation failed!" << std::endl;
        exit(-1);
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); // Clear structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(clientfd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Connection to server failed!" << std::endl;
        close(clientfd);
        exit(-1);
    }

    for (;;)
    {

        std::cout << "=================== Main Menu ===================" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;
        std::cout << "3. Quit" << std::endl;
        std::cout << "=================================================" << std::endl;
        std::cout << "Please enter your choice: ";
        int choice = 0;
        std::cin >> choice;
        std::cin.get(); // Clear newline character from input buffer

        switch (choice)
        {
        case 1:
        {
            // Login
            int id = 0;
            char pwd[50] = {0};
            std::cout << "Please enter user ID: ";
            std::cin >> id;
            std::cin.get(); // Clear newline character
            std::cout << "Please enter password: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
            if (len == -1)
            {
                std::cerr << "Send login request failed!" << std::endl;
            }
            else
            {
                char buffer[1024] = {0};
                int len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                {
                    std::cerr << "Receive login response failed!" << std::endl;
                }
                else
                {

                    json response = json::parse(buffer);
                    if (response["errno"].get<int>() == 0)
                    {
                        g_currentUser.setId(response["id"].get<int>());
                        g_currentUser.setName(response["name"]);
                        g_currentUser.setState("online");

                        // Display friend list
                        if (response.contains("friends"))
                        {

                            for (json &friendjs : response["friends"])
                            {
                                User frienduser;
                                frienduser.setId(friendjs["id"].get<int>());
                                frienduser.setName(friendjs["name"]);
                                frienduser.setState(friendjs["state"]);
                                g_currentUserFriendList.push_back(frienduser);
                            }
                        }
                        // Display group list
                        if (response.contains("groups"))
                        {
                            for (json &groupjs : response["groups"])
                            {
                                Group group;
                                group.setId(groupjs["id"].get<int>());
                                group.setName(groupjs["name"]);
                                group.setDesc(groupjs["desc"]);

                                for (json &userjs : groupjs["users"])
                                {
                                    GroupUser user;
                                    user.setId(userjs["id"].get<int>());
                                    user.setName(userjs["name"]);
                                    user.setState(userjs["state"]);
                                    group.addUser(user);
                                }

                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        showCurrentUserData();

                        if (response.contains("offlinemsg"))
                        {
                            std::cout << "You have offline messages:" << std::endl;
                            for (json &msg : response["offlinemsg"])
                            {
                                std::cout << msg["from"].get<std::string>() << " said: " << msg["message"].get<std::string>() << std::endl;
                            }
                        }

                        std::thread readTask(readTaskHandler, clientfd);
                        readTask.detach();

                        mainMenu();
                    }
                    else
                    {
                        std::cout << "Login failed! errno: " << response["errno"].get<int>() << std::endl;
                    }
                }
            }

            break;
        }
        case 2:
        {
            // Register
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "Please enter username: ";
            std::cin.getline(name, 50);
            std::cout << "Please enter password: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);

            if (len == -1)
            {
                std::cerr << "Send registration request failed!" << std::endl;
            }
            else
            {
                char buffer[1024] = {0};
                int len = recv(clientfd, buffer, 1024, 0);
                if (len == -1)
                {
                    std::cerr << "Receive registration response failed!" << std::endl;
                }
                else
                {
                    json response = json::parse(buffer);
                    if (response["errno"].get<int>() == 0)
                    {
                        std::cout << "Registration successful! Your user ID is: " << response["id"].get<int>() << std::endl;
                    }
                    else
                    {
                        std::cout << "Registration failed! errmsg: " << response["errmsg"].get<std::string>() << std::endl;
                    }
                }
            }

            break;
        }
        case 3:
        {
            std::cout << "Quitting the application." << std::endl;
            close(clientfd);
            exit(0);
        }
        default:
        {
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
        }
    }

    return 0;
}

void showCurrentUserData()
{
    std::cout << "=================== User Information ===================" << std::endl;
    std::cout << "User ID: " << g_currentUser.getId() << std::endl;
    std::cout << "Username: " << g_currentUser.getName() << std::endl;
    std::cout << "State: " << g_currentUser.getState() << std::endl;
    std::cout << "=================== Friend List ===================" << std::endl;
    for (const auto &friendUser : g_currentUserFriendList)
    {
        std::cout << "Friend ID: " << friendUser.getId()
                  << ", Name: " << friendUser.getName()
                  << ", State: " << friendUser.getState() << std::endl;
    }
    std::cout << "=================== Group List ===================" << std::endl;
    for (const auto &group : g_currentUserGroupList)
    {
        std::cout << "Group ID: " << group.getId()
                  << ", Name: " << group.getName()
                  << ", Description: " << group.getDesc() << std::endl;
        for (const auto &user : group.getUsers())
        {
            std::cout << "    User ID: " << user.getId()
                      << ", Name: " << user.getName()
                      << ", State: " << user.getState() << std::endl;
        }
    }
    std::cout << "======================================================" << std::endl;
}

void readTaskHandler(int clientfd)
{
}

string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(buffer);
}

void mainMenu()
{
}