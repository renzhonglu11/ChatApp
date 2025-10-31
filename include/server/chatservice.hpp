#pragma once

#include <unordered_map>
#include <functional>

#include "userModel.hpp"
#include <muduo/net/TcpServer.h>
#include "json.hpp"

using json = nlohmann::json;
using namespace muduo::net;
using namespace muduo;

// Handle message event callback type
using MsgHandler = std::function<void(const TcpConnectionPtr &, json &, Timestamp)>;

// Chat service class definition
class ChatService
{
public:
    static ChatService *instance(); // singleton
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    MsgHandler getHandler(int msgid);

private:
    ChatService();
    std::unordered_map<int, MsgHandler> _msgHandlerMap;

    UserModel _userModel;
};