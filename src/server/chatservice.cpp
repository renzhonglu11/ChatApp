#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
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
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "user logins in...";
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