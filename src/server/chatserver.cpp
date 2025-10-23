#include "chatserver.hpp"
#include <functional>

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
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
}