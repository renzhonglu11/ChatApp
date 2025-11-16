#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <muduo/base/Logging.h>

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
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime)
{
    string msg = buf->retrieveAllAsString();

    // Filter out non-json format messages
    bool hasControl = false;
    for (unsigned char c : msg)
    {
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t')
        {
            hasControl = true;
            break;
        }
    }
    if (msg.empty() || hasControl)
    {
        LOG_WARN << "Discarding non-JSON/control payload size=" << msg.size();
        return; // do not parse
    }

    // Quick JSON accept check
    if (!json::accept(msg))
    {
        LOG_WARN << "Invalid JSON text: " << msg;
        return;
    }
    // conn->send(msg);

    try
    {
        json js = json::parse(msg);

        // conn->send(js.dump()); // For debug

        // 通过js["msgid"]来获业务handler -> conn, js, time
        // The code does not need to be changed when the service logic is changed
        auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
        msgHandler(conn, js, receiveTime);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "JSON/handler error: " << e.what();
    }
}
