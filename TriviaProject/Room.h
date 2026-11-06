#pragma once
#include "LoggedUser.h"
#include <vector>
#include <string>

enum RoomStatus
{
    WAITING,
    ACTIVE,
    CLOSED
};

struct RoomData
{
    unsigned int id = 0;
    std::string name;
    std::string creator;
    unsigned int maxPlayers = 0;
    unsigned int numOfQuestionsInGame = 10;
    unsigned int timePerQuestion = 30;
    RoomStatus status = WAITING;
};

class Room {
public:
    Room(RoomData data);
    ~Room() = default;

    void addUser(LoggedUser user);
    void removeUser(LoggedUser user);
    std::vector<std::string> getAllUsers();
    RoomData getRoomData();
    void setStatus(RoomStatus status);
private:
    bool userExists(std::string username);
    RoomData m_metadata;
    std::vector<LoggedUser> m_users;
};