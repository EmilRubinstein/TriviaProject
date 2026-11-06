#include "RoomManager.h"
#include <stdexcept>
#include <iostream>

RoomManager::RoomManager()
{}

void RoomManager::createRoom(LoggedUser user, RoomData r_data)
{
    Room room(r_data);
    room.addUser(user);
    m_rooms.insert({ r_data.id, room });
}

void RoomManager::deleteRoom(int ID)
{
    m_rooms.erase(ID);

}

int RoomManager::getRoomState(int ID)
{
    auto it = m_rooms.find(ID);
    if (it == m_rooms.end())
    {
        throw std::runtime_error("Room not found");
    }
    return it->second.getRoomData().status;
}

std::vector<int> RoomManager::getRooms()
{
    std::vector<int> rooms;
    std::vector<int> roomsToDelete;

    for (auto& it : m_rooms)
    {
        if (it.second.getAllUsers().empty())
        {
            roomsToDelete.push_back(it.first);
        }
        else
        {
            rooms.push_back(it.first);
        }
    }

    for (int roomId : roomsToDelete)
    {
        m_rooms.erase(roomId);
        std::cout << "[RoomManager] Deleted empty room " << roomId << std::endl;
    }

    return rooms;
}

Room& RoomManager::getRoom(int ID)
{
    auto it = m_rooms.find(ID);
    if (it == m_rooms.end())
    {
        throw std::runtime_error("Room not found");
    }
    return it->second;
}
void RoomManager::cleanupEmptyRooms()
{
    std::vector<int> roomsToDelete;

    for (auto& pair : m_rooms)
    {
        if (pair.second.getAllUsers().empty())
        {
            roomsToDelete.push_back(pair.first);
        }
    }

    for (int roomId : roomsToDelete)
    {
        m_rooms.erase(roomId);
        std::cout << "[RoomManager] Deleted empty room " << roomId << std::endl;
    }
}