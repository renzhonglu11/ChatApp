#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer
{

public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name);

    void start();

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime);

    EventLoop *_loop;
    TcpServer _server;
};
