#pragma once
#include "Communicator.h"
#include "LoginManager.h"
#include "SqliteDataBase.h"

class Server {
public:
    static Server& getInstance(int port = 8080)
    {
        static Server instance(port);
        return instance;
    }

    // Delete copy & assign
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    ~Server();
    void run();

private:
    Server(int port);

    int m_port;
    void consoleListener();
    std::atomic<bool> _running;
};