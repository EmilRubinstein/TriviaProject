#pragma once
#include <map>
#include "Room.h"

class RoomManager {
public:
    static RoomManager& getInstance() {
        static RoomManager instance;
        return instance;
    }

    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;

    void startAutoCleanup();
    void cleanupEmptyRooms();
    void createRoom(LoggedUser user, RoomData r_data);
    void deleteRoom(int ID);
    int getRoomState(int ID);
    std::vector<int > getRooms();
    Room& getRoom(int ID);
    bool roomExists(int ID) {
        return m_rooms.find(ID) != m_rooms.end();
    }

private:
    RoomManager();
    std::map <int, Room> m_rooms;

};