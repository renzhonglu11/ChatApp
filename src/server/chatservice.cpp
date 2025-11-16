#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>
#include <vector>

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

void ChatService::reset()
{
    // reset all user state to offline
    _userModel.resetState();
}

ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2,
                                                std::placeholders::_3)});

    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this,
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              std::placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this,
                                                   std::placeholders::_1,
                                                   std::placeholders::_2,
                                                   std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this,
                                                     std::placeholders::_1,
                                                     std::placeholders::_2,
                                                     std::placeholders::_3)});
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id); // decouple database operation

    if (user.getPassword() == password && user.getId() == id)
    {
        LOG_INFO << "user state: " << user.getState();
        if (user.getState() == "online")
        {
            bool hasConn = false;
            {
                std::lock_guard<std::mutex> lock(_connMutex);
                hasConn = (_userConnMap.find(id) != _userConnMap.end());
            }
            if (!hasConn)
            {
                LOG_WARN << "Stale online state for user " << id << ", forcing relogin.";
                {
                    std::lock_guard<std::mutex> lock(_connMutex);
                    _userConnMap[id] = conn;
                }
            }

            // already online
            LOG_INFO << "user id: " << id << " already online, login failed!";
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // already online
            response["erromsg"] = "user already online!";
            conn->send(response.dump());
        }
        else
        {
            // login success
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // state offline -> online
            user.setState("online");
            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // success
            response["id"] = user.getId();
            response["name"] = user.getName();

            // Check whether there are offline messages
            vector<string> offlinemsgs = _offLineMsgModel.query(id);
            if (!offlinemsgs.empty())
            {
                response["offlinemsg"] = offlinemsgs;
                // Delete offline messages from database
                _offLineMsgModel.remove(id);
            }

            // Check friend list
            vector<User> friends = _friendModel.queryFriends(id);
            if (!friends.empty())
            {
                vector<json> friendVec;
                for (User &frienduser : friends)
                {
                    json friendjs;
                    friendjs["id"] = frienduser.getId();
                    friendjs["name"] = frienduser.getName();
                    friendjs["state"] = frienduser.getState();
                    friendVec.push_back(friendjs);
                }
                response["friends"] = friendVec;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // login failed
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // failed
        response["id"] = user.getId();
        response["name"] = user.getName();
        response["errmsg"] = "password error!";
        conn->send(response.dump());
    }
}
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user); // decouple database operation
    json response;

    if (state)
    {
        // registration success
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // success

        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // registration failed
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    // logging msgid is not valid
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }
    return _msgHandlerMap[msgid];
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;

    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // found the user connection
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    user.setState("offline");
    _userModel.updateState(user);
    LOG_INFO << "user id: " << user.getId() << " offline handled!";
}

// one to one chat
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toId = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end())
        {
            // toId is online, forward the message
            it->second->send(js.dump());
            return;
        }
    }

    // offline message
    _offLineMsgModel.insert(toId, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();

    if (userId != friendId && _userModel.query(friendId).getId() != -1 && _userModel.query(userId).getId() != -1)
    {
        // Store the friend relationship in the database
        _friendModel.addFriend(userId, friendId);
        LOG_INFO << "User " << userId << " added friend " << friendId;
        return;
    }

    LOG_ERROR << "Add friend failed: invalid user IDs " << userId << " or " << friendId;
}