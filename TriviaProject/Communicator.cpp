#include "Communicator.h"
#include "RequestHandlerFactory.h"
#include "LoginRequestHandler.h"   
#include "SqliteDataBase.h"      
#include "RoomManager.h"
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include "MenuRequestHandler.h"

#pragma comment(lib, "ws2_32.lib")

//------------------------------------------------------BASICS---------------------------------------------------
Communicator::Communicator(int port, LoginManager& loginManager) : _port(port), _running(true), m_loginManager(&loginManager)
{
    // Initialize WinSock library (required before any socket operations)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "[Error] WSAStartup failed: " << result << std::endl;
        exit(1);
    }

    // Create TCP socket for server
    _serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_serverSocket == INVALID_SOCKET) {
        std::cerr << "[Error] socket() failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    // Bind socket to local address and port
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[Error] bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(_serverSocket);
        WSACleanup();
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[Error] listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(_serverSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "[Server] Listening on port " << _port << std::endl;
}

Communicator::~Communicator() {
    _running = false;

    // Close main server socket
    closesocket(_serverSocket);

    std::lock_guard<std::mutex> lock(_clientsMutex);

    // Close all active client sockets and free handlers
    for (auto& [sock, handler] : _clients) {
        closesocket(sock);
        delete handler;
    }

    _clients.clear();
    WSACleanup();
}

//------------------------------------------------------BASICS---------------------------------------------------
void Communicator::stop()
{
    _running = false;
    closesocket(_serverSocket);
}

void Communicator::startHandleRequests()
{
    while (_running) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);

        // Accept new incoming client connection
        SOCKET clientSocket = accept(_serverSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            if (_running) {
                std::cerr << "[Error] accept() failed: " << WSAGetLastError() << std::endl;
            }
            continue;
        }

        // Convert client IP to readable string
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));

        std::cout << "[Server] New client connected: " << clientIP
            << " (socket " << clientSocket << ")" << std::endl;

        {
            std::lock_guard<std::mutex> lock(_clientsMutex);

            if (m_loginManager != nullptr) {
                _clients[clientSocket] = new LoginRequestHandler(LoginManager::getInstance(), RequestHandlerFactory::getInstance(&SqliteDataBase::getInstance()));
            }
            else {
                std::cerr << "[Error] No LoginManager available!" << std::endl;
                closesocket(clientSocket);
                continue;
            }
        }

        // Create dedicated thread per client
        std::thread t(&Communicator::handleNewClient, this, clientSocket);
        t.detach();
    }
}

void Communicator::handleNewClient(SOCKET clientSocket)
{
    std::cout << "[Server] Handler thread started for socket " << clientSocket << "\n";

    while (true)
    {
        RequestInfo req;

        try
        {
            // Blocking read of a full request packet
            req = readRequest(clientSocket);

            std::cout << "_____________________(client socket:" << clientSocket << ")____________________\n";
        }
        catch (const std::exception& e)
        {
            // Triggered when client disconnects or socket breaks
            std::cout << "[Server] Client disconnected (socket "
                << clientSocket << "): " << e.what() << "\n";
            break;
        }

        printRequest(req);

        IRequestHandler* handler = nullptr;

        {
            std::lock_guard<std::mutex> lock(_clientsMutex);

            auto it = _clients.find(clientSocket);

            if (it == _clients.end() || it->second == nullptr)
            {
                std::cout << "[Server] Invalid handler\n";
                return;
            }

            handler = it->second;
        }

        // Process request and generate response
        RequestResult result = handler->handleRequest(req);

        printResponse(result);

        // Send raw serialized response back to client
        int sent = send(
            clientSocket,
            reinterpret_cast<const char*>(result.response.data()),
            (int)result.response.size(),
            0
        );

        if (sent == SOCKET_ERROR)
        {
            std::cout << "[Server] send() failed, closing client\n";
            break;
        }

        // Optional handler transition (state machine pattern)
        if (result.newHandler != nullptr)
        {
            std::lock_guard<std::mutex> lock(_clientsMutex);

            if (result.newHandler != _clients[clientSocket])
            {
                delete _clients[clientSocket];
                _clients[clientSocket] = result.newHandler;
            }
        }

        std::cout << "_________________________________________\n";
    }

    closeClient(clientSocket);
}

RequestInfo Communicator::readRequest(SOCKET clientSocket)
{
    RequestInfo req;

    // Read request code (1 byte)
    int received = readFully(clientSocket, (char*)&req.code, 1);
    if (received <= 0)
        throw std::runtime_error("socket closed");

    // Read payload size (4 bytes)
    int size = 0;
    received = readFully(clientSocket, (char*)&size, 4);
    if (received <= 0)
        throw std::runtime_error("socket closed");

    // Allocate buffer for JSON payload
    req.json_raw_data.resize(size);

    int total = 0;

    // Ensure full payload is received (TCP may split packets)
    while (total < size)
    {
        int r = recv(
            clientSocket,
            (char*)req.json_raw_data.data() + total,
            size - total,
            0
        );

        if (r <= 0)
            throw std::runtime_error("socket closed");

        total += r;
    }

    req.time = time(nullptr);
    return req;
}

void Communicator::closeClient(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> lock(_clientsMutex);

    auto it = _clients.find(clientSocket);

    if (it != _clients.end())
    {
        IRequestHandler* handler = it->second;

        MenuRequestHandler* menuHandler = dynamic_cast<MenuRequestHandler*>(handler);

        if (menuHandler != nullptr)
        {
            std::string username = menuHandler->getUser().getUsername();

            std::vector<int> rooms = RoomManager::getInstance().getRooms();

            for (int roomId : rooms)
            {
                try
                {
                    Room& room = RoomManager::getInstance().getRoom(roomId);
                    RoomData roomData = room.getRoomData();
                    std::vector<std::string> players = room.getAllUsers();

                    bool isAdmin = (roomData.creator == username);

                    for (const std::string& player : players)
                    {
                        if (player == username)
                        {
                            LoggedUser user(username);
                            room.removeUser(user);
                            std::cout << "[Server] Removed " << username << " from room " << roomId << std::endl;

                            // If admin disconnected or room empty, delete the room
                            if (isAdmin || room.getAllUsers().empty())
                            {
                                std::cout << "[Server] Admin disconnected or room empty - deleting room " << roomId << std::endl;
                                RoomManager::getInstance().deleteRoom(roomId);
                            }
                            break;
                        }
                    }
                }
                catch (...)
                {
                    // Room doesn't exist or other error - ignore
                }
            }

            m_loginManager->logout(username);
        }

        closesocket(it->first);
        delete handler;
        _clients.erase(it);

        std::cout << "[Server] Client " << clientSocket << " disconnected." << std::endl;
    }
}