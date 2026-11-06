#include "Server.h"
#include "Communicator.h"
#include "LoginManager.h"
#include "SqliteDataBase.h"
#include "RequestHandlerFactory.h"

Server::Server(int port)
    : m_port(port), _running(true)
{
    //initialize all singletons in right order:
    SqliteDataBase::getInstance();                              
    LoginManager::getInstance(&SqliteDataBase::getInstance());
    Communicator::getInstance(port, LoginManager::getInstance()); 
    RequestHandlerFactory::getInstance(&SqliteDataBase::getInstance());
}


Server::~Server() {}

void Server::run()
{
    std::thread console(&Server::consoleListener, this);
    console.detach();
    Communicator::getInstance(m_port, LoginManager::getInstance()).startHandleRequests();
}

void Server::consoleListener()
{
    std::string input;
    std::cout << "[Console] Type EXIT to shut down the server." << std::endl;
    while (_running && std::getline(std::cin, input)) {
        if (input == "EXIT") {
            std::cout << "[Server] Shutting down..." << std::endl;
            _running = false;
            Communicator::getInstance(m_port, LoginManager::getInstance()).stop();
            break;
        }
        else {
            std::cout << "[Console] Unknown command: " << input << std::endl;
        }
    }
}