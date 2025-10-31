#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
#include <iostream>

using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : _server(loop, listenAddr, name),
      _loop(loop)
{
    _server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, std::placeholders::_1));

    _server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));

    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    string msg = buf->retrieveAllAsString();
    conn->send(msg);
    json js = json::parse(msg);

    // 通过js["msgid"]来获业务handler -> conn, js, time
    // The code does not need to be changed when the service logic is changed
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, receiveTime);
}