#include "chatserver.hpp"
#include <iostream>
#include <signal.h>
#include "chatservice.hpp"

void resetHandler(int)
{
    std::cout << "\nServer is shutting down. Resetting all user states..." << std::endl;
    ChatService::instance()->reset();
    exit(0);
}

int main()
{

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 8888);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
}