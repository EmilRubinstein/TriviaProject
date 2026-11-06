#pragma once
#include "IRequestHandler.h"
#include "IDatabase.h"
#include "LoginManager.h"
#include "RoomManager.h"
#include "StatisticsManager.h"
#include "LoggedUser.h"
#include "Room.h"
#include "GameManager.h"

class RequestHandlerFactory
{
public:
    static RequestHandlerFactory& getInstance(IDatabase* database = nullptr) {
        static RequestHandlerFactory instance(database);
        return instance;
    }
    RequestHandlerFactory(const RequestHandlerFactory&) = delete;
    RequestHandlerFactory& operator=(const RequestHandlerFactory&) = delete;

    IRequestHandler* createLoginRequestHandler();
    IRequestHandler* createMenuRequestHandler(LoggedUser user);
    IRequestHandler* createRoomAdminRequestHandler(Room room, LoggedUser user);
    IRequestHandler* createRoomMemberRequestHandler(Room room, LoggedUser user);
    IRequestHandler* createGameRequestHandler(LoggedUser user, int roomId);

    LoginManager& getLoginManager();
    StatisticsManager& getStatisticsManager();
    RoomManager& getRoomManager();
    IDatabase& getDatabase();
    GameManager& getGameManager();

private:
    explicit RequestHandlerFactory(IDatabase* database);
    IDatabase* m_database;
};