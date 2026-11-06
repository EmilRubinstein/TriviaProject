#pragma once
#include <iostream>
#include <String>

#include "Helper.h"
#include "IRequestHandler.h"
#include "LoginRequestHandler.h"

#include <WinSock2.h>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

class LoginManager;

class Communicator {
public:
    static Communicator& getInstance(int port, LoginManager& loginManager)
    {
        static Communicator instance(port, loginManager); 
        return instance;
    }

    Communicator(const Communicator&) = delete;
    Communicator& operator=(const Communicator&) = delete;

    ~Communicator();
    void startHandleRequests();
    void stop();

private:
    Communicator(int port, LoginManager& loginManager); 
    void handleNewClient(SOCKET clientSocket);
    void closeClient(SOCKET clientSocket);
    RequestInfo readRequest(SOCKET clientSocket);

    SOCKET _serverSocket;
    int _port;
    std::atomic<bool> _running;
    std::map<SOCKET, IRequestHandler*> _clients;
    std::mutex _clientsMutex;
    LoginManager* m_loginManager;
};