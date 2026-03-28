#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>

#include "friendModel.hpp"
#include "json.hpp"
#include "offLineMsgModel.hpp"
#include "userModel.hpp"
#include <muduo/net/TcpServer.h>

#include "group.hpp"
#include "groupModel.hpp"

#include "redis.hpp"

using json = nlohmann::json;
using namespace muduo::net;
using namespace muduo;

// Handle message event callback type
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

// Chat service class definition
class ChatService {
public:
    static ChatService* instance(); // singleton
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    MsgHandler getHandler(int msgid);

    // Add friends
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void clientCloseException(const TcpConnectionPtr& conn);

    void logout(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void handleRedisSubscribeMessage(int userId, string msg);

    // reset all user state to offline
    void reset();

private:
    ChatService();
    std::unordered_map<int, MsgHandler> _msgHandlerMap;
    // Store online user's connection
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    UserModel _userModel;
    OffLineMsgModel _offLineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    std::mutex _connMutex; // mutex for protecting _userConnMap

    // Redis
    Redis _redis;
};