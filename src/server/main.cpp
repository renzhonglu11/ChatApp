#include "chatserver.hpp"
#include <iostream>
#include <signal.h>
#include <string>
#include <cstdlib>
#include "chatservice.hpp"

void resetHandler(int)
{
    std::cout << "\nServer is shutting down. Resetting all user states..." << std::endl;
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, resetHandler);

    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
        std::cerr << "Example: " << argv[0] << " 127.0.0.1 8888\n";
        return 1;
    }

    std::string ip = argv[1];
    int port = std::atoi(argv[2]);

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port: " << argv[2] << ". Port must be in range 1-65535.\n";
        return 1;
    }

    EventLoop loop;
    InetAddress addr(ip, static_cast<uint16_t>(port));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}