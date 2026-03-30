#include "json.hpp"
#include <chrono>
#include <ctime>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::json;

#include <arpa/inet.h>
#include <atomic>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "group.hpp"
#include "public.hpp"
#include "user.hpp"

#include <semaphore.h>

User g_currentUser; // Store the current logged-in user information
std::vector<User> g_currentUserFriendList; // Store the current user's friend list
std::vector<Group> g_currentUserGroupList; // Store the current user's group list
std::atomic<bool> g_running { true };
std::atomic<bool> g_isMenuRunning { false };

std::atomic<bool> g_isLoginSuccess { false };
sem_t rw_sm;

enum class MainMenuChoice {
    Login = 1,
    Register = 2,
    Quit = 3,
    Invalid
};

#include <functional>
#include <unordered_map>

void help(int fd = 0, string str = "");
void chat(int, string);
void addFriend(int, string);
void createGroup(int, string);
void addGroup(int, string);
void groupChat(int, string);
void logout(int, string);

unordered_map<string, string> commandMap = {
    { "help", "Show available commands, format: help" },
    { "chat", "Send a one-on-one chat message, format: chat:friendid:message" },
    { "addfriend", "Add a friend by user ID, format: addfriend:friendid" },
    { "creategroup", "Create a new group, format: creategroup:groupname:groupdesc" },
    { "addgroup", "Join an existing group, format: addgroup:groupid" },
    { "groupchat", "Send a group chat message, format: groupchat:groupid:message" },
    { "logout", "Logout from the application, format: logout" },
};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    { "help", help },
    { "chat", chat },
    { "addfriend", addFriend },
    { "creategroup", createGroup },
    { "addgroup", addGroup },
    { "groupchat", groupChat },
    { "logout", logout },
};

void mainMenu(int clientfd)
{
    help();
    char commandBuf[100] = { 0 };
    std::cout << "Please enter command: " << std::endl;
    while (g_isMenuRunning.load()) {
        std::cin.getline(commandBuf, 100);
        string commandStr(commandBuf);
        auto pos = commandStr.find(':');
        string command;
        string params;
        if (pos == string::npos) // e.g. help, logout
        {
            command = commandStr;
        } else {
            command = commandStr.substr(0, pos);
            params = commandStr.substr(pos + 1);
        }

        auto it = commandHandlerMap.find(command);
        if (it != commandHandlerMap.end()) {
            it->second(clientfd, params); // Call the corresponding command handler
        } else {
            std::cout << "Unknown command! Type 'help' to see available commands." << std::endl;
        }
    }
}

void showCurrentUserData();
void readTaskHandler(int clientfd);
string getCurrentTime();
int createClientSocket(const char* ip, uint16_t port);
void runClient(int clientfd);
MainMenuChoice showMainMenuAndGetChoice();
void handleLogin(int clientfd);
void handleRegister(int clientfd);
void handleQuit(int clientfd);

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout << "Usage: ./ChatClient <IP> <Port>" << std::endl;
        exit(-1);
    }

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = createClientSocket(ip, port);
    if (clientfd == -1) {
        exit(-1);
    }

    runClient(clientfd);

    return 0;
}

int createClientSocket(const char* ip, uint16_t port)
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);

    if (connect(clientfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Connection to server failed!" << std::endl;
        close(clientfd);
        return -1;
    }

    return clientfd;
}

void runClient(int clientfd)
{
    // Initialize semaphore, 0 for shared between threads, initial value 0
    sem_init(&rw_sm, 0, 0);

    // Start a separate thread to read messages from the server
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    while (g_running.load()) {
        MainMenuChoice choice = showMainMenuAndGetChoice();

        switch (choice) {
        case MainMenuChoice::Login:
            handleLogin(clientfd);
            break;
        case MainMenuChoice::Register:
            handleRegister(clientfd);
            break;
        case MainMenuChoice::Quit:
            handleQuit(clientfd);
            // handleQuit will exit, but break for clarity
            break;
        case MainMenuChoice::Invalid:
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
    }
}

MainMenuChoice showMainMenuAndGetChoice()
{
    std::cout << "=================== Main Menu ===================" << std::endl;
    std::cout << "1. Login" << std::endl;
    std::cout << "2. Register" << std::endl;
    std::cout << "3. Quit" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << "Please enter your choice: ";

    int choice = 0;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return MainMenuChoice::Invalid;
    }
    std::cin.get(); // Clear newline character from input buffer

    switch (choice) {
    case 1:
        return MainMenuChoice::Login;
    case 2:
        return MainMenuChoice::Register;
    case 3:
        return MainMenuChoice::Quit;
    default:
        return MainMenuChoice::Invalid;
    }
}

void handleLogin(int clientfd)
{
    int id = 0;
    char pwd[50] = { 0 };
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

    g_isLoginSuccess.store(false);

    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send login request failed!" << std::endl;
        return;
    }

    sem_wait(&rw_sm); // wait for login response to be processed in readTaskHandler, which will post rw_sm after processing login response

    if (g_isLoginSuccess.load()) {
        std::cout << "Login successful!" << std::endl;
        g_isMenuRunning.store(true);
        mainMenu(clientfd);

    } else {
        std::cout << "Login failed!" << std::endl;
        g_isMenuRunning.store(false);
    }
}

void handleRegisterResponse(json& response)
{
    if (response["errno"].get<int>() == 0) {
        std::cout << "Registration successful! Your user ID is: " << response["id"].get<int>() << std::endl;
    } else {
        std::cout << "Registration failed! errmsg: " << response["errmsg"].get<std::string>() << std::endl;
    }
}

void handleRegister(int clientfd)
{
    char name[50] = { 0 };
    char pwd[50] = { 0 };
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
    if (len == -1) {
        std::cerr << "Send registration request failed!" << std::endl;
        return;
    }

    sem_wait(&rw_sm);
}

void handleQuit(int clientfd)
{
    std::cout << "Quitting the application." << std::endl;
    close(clientfd);
    sem_destroy(&rw_sm);
    exit(0);
}

void showCurrentUserData()
{
    std::cout << "=================== User Information ===================" << std::endl;
    std::cout << "User ID: " << g_currentUser.getId() << std::endl;
    std::cout << "Username: " << g_currentUser.getName() << std::endl;
    std::cout << "State: " << g_currentUser.getState() << std::endl;
    std::cout << "=================== Friend List ===================" << std::endl;
    for (const auto& friendUser : g_currentUserFriendList) {
        std::cout << "Friend ID: " << friendUser.getId()
                  << ", Name: " << friendUser.getName()
                  << ", State: " << friendUser.getState() << std::endl;
    }
    std::cout << "=================== Group List ===================" << std::endl;
    for (const auto& group : g_currentUserGroupList) {
        std::cout << "Group ID: " << group.getId()
                  << ", Name: " << group.getName()
                  << ", Description: " << group.getDesc() << std::endl;
        for (const auto& user : group.getUsers()) {
            std::cout << "    User ID: " << user.getId()
                      << ", Name: " << user.getName()
                      << ", State: " << user.getState()
                      << ", Role: " << user.getRole() << std::endl;
        }
    }
    std::cout << "======================================================" << std::endl;
}

// Handle login logic
void handleLoginResponse(json& response)
{

    if (response["errno"].get<int>() == 0) {
        g_currentUser.setId(response["id"].get<int>());
        g_currentUser.setName(response["name"]);
        g_currentUser.setState("online");

        // Display friend list
        if (response.contains("friends")) {
            g_currentUserFriendList.clear();

            for (json& friendjs : response["friends"]) {
                User frienduser;
                frienduser.setId(friendjs["id"].get<int>());
                frienduser.setName(friendjs["name"]);
                frienduser.setState(friendjs["state"]);
                g_currentUserFriendList.push_back(frienduser);
            }
        }

        // Display group list
        if (response.contains("groups")) {

            g_currentUserGroupList.clear();

            for (json& groupjs : response["groups"]) {
                Group group;
                group.setId(groupjs["id"].get<int>());
                group.setName(groupjs["name"]);
                group.setDesc(groupjs["desc"]);

                for (json& userjs : groupjs["users"]) {
                    GroupUser user;
                    user.setId(userjs["id"].get<int>());
                    user.setName(userjs["name"]);
                    user.setState(userjs["state"]);
                    if (userjs.contains("role")) {
                        user.setRole(userjs["role"]);
                    }
                    group.addUser(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        showCurrentUserData();

        if (response.contains("offlinemsg")) {
            std::cout << "You have offline messages:" << std::endl;
            for (const auto& msgItem : response["offlinemsg"]) {
                json msg = json::parse(msgItem.get<std::string>()); // Parse string to json
                int msgType = msg["msgid"].get<int>();
                if (msgType == ONE_CHAT_MSG) {
                    std::cout << "At time " << msg["time"].get<std::string>() << ", " << msg["from"].get<std::string>() << "[" << msg["id"].get<int>() << "]"
                              << " said: " << msg["message"].get<std::string>() << std::endl;
                } else if (msgType == GROUP_CHAT_MSG) {
                    std::cout << "At time " << msg["time"].get<std::string>() << " [Group " << msg["groupid"].get<int>() << "] [" << msg["from"].get<std::string>() << "[" << msg["id"].get<int>() << "]"
                              << " said: " << msg["message"].get<std::string>() << std::endl;
                }
            }
        }

        g_isLoginSuccess.store(true);

    } else {
        std::cout << "Login failed! errno: " << response["errno"].get<int>() << std::endl;
        g_isLoginSuccess.store(false);
    }
}

// Read message from server
void readTaskHandler(int clientfd)
{
    for (;;) {
        char buffer[1024] = { 0 };
        int len = recv(clientfd, buffer, sizeof(buffer), 0); // block
        if (len == -1 || len == 0) {
            std::cerr << "Disconnected from server." << std::endl;
            g_running.store(false);
            g_isMenuRunning.store(false);
            break;
        }

        json js = json::parse(buffer);
        auto msgType = js["msgid"].get<int>();
        if (msgType == ONE_CHAT_MSG) {
            std::cout << js["time"].get<std::string>() << " [" << js["from"].get<std::string>() << "] said: " << js["message"].get<std::string>() << std::endl;
        } else if (msgType == GROUP_CHAT_MSG) {
            std::cout << js["time"].get<std::string>() << " [Group " << js["groupid"].get<int>() << "] [" << js["from"].get<std::string>() << "[" << js["id"].get<int>() << "]"
                      << " said: " << js["message"].get<std::string>() << std::endl;
        } else if (msgType == LOGIN_MSG_ACK) {
            handleLoginResponse(js);
            sem_post(&rw_sm); // inform the main thread that login response has been processed.
            continue;
        } else if (msgType == REG_MSG_ACK) {
            handleRegisterResponse(js);
            sem_post(&rw_sm); // inform the main thread that register response has been processed.
            continue;
        } else {
            std::cout << "Received unknown message type: " << msgType << std::endl;
        }
    }
}

string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_c));
    return std::string(buffer);
}

void help(int, string)
{
    std::cout << "=================== Available Commands ===================" << std::endl;
    for (const auto& cmd : commandMap) {
        std::cout << cmd.first << " : " << cmd.second << std::endl;
    }
    std::cout << "======================================================" << std::endl;
}

void addFriend(int clientfd, string params)
{
    int friendId = stoi(params);

    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendId;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send add friend request failed!" << std::endl;
        return;
    }
    std::cout << "Add friend request sent to user successfully!" << friendId << std::endl;
}

void chat(int clientfd, string params)
{
    auto pos = params.find(':');
    if (pos == string::npos) {
        std::cout << "Invalid format! Correct format: chat:friendid:message" << std::endl;
        return;
    }
    int friendId = stoi(params.substr(0, pos));
    string message = params.substr(pos + 1);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["from"] = g_currentUser.getName();
    js["id"] = g_currentUser.getId();
    js["to"] = friendId;
    js["message"] = message;
    js["time"] = getCurrentTime();

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send chat message failed!" << std::endl;
    }
}

void createGroup(int clientfd, string params)
{
    auto idx = params.find(':');
    if (idx == -1) {
        std::cout << "Invalid format! Correct format: creategroup:groupname:groupdesc" << std::endl;
        return;
    }

    string groupName = params.substr(0, idx);
    string groupDesc = params.substr(idx + 1);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupName;
    js["groupdesc"] = groupDesc;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send create group request failed!" << std::endl;
    }
};
void addGroup(int clientfd, string params)
{
    int groupId = stoi(params);

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupId;

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send add group request failed!" << std::endl;
    }
};
void groupChat(int clientfd, string params)
{
    auto idx = params.find(':');
    if (idx == -1) {
        std::cout << "Invalid format! Correct format: groupchat:groupid:message" << std::endl;
        return;
    }

    int groupId = stoi(params.substr(0, idx));
    string message = params.substr(idx + 1);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["from"] = g_currentUser.getName();
    js["message"] = message;
    js["groupid"] = groupId;
    js["time"] = getCurrentTime();

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send group chat message failed!" << std::endl;
    }
};
void logout(int clientfd, string params)
{
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();

    string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if (len == -1) {
        std::cerr << "Send logout request failed!" << std::endl;
    } else {
        std::cout << "Logout request sent successfully!" << std::endl;
        g_isMenuRunning.store(false);
    }
};
