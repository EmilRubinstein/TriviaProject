#include "RequestHandlerFactory.h"
#include "LoginRequestHandler.h"
#include "MenuRequestHandler.h"
#include "RoomAdminRequestHandler.h"
#include "RoomMemberRequestHandler.h"
#include "GameRequestHandler.h"
#include "GameManager.h"

RequestHandlerFactory::RequestHandlerFactory(IDatabase* database)
    : m_database(database)
{}

IRequestHandler* RequestHandlerFactory::createLoginRequestHandler()
{
    return new LoginRequestHandler(LoginManager::getInstance(), *this);
}

IRequestHandler* RequestHandlerFactory::createMenuRequestHandler(LoggedUser user)
{
    return new MenuRequestHandler(user, *this);
}

IRequestHandler* RequestHandlerFactory::createRoomAdminRequestHandler(Room room, LoggedUser user)
{
    return new RoomAdminRequestHandler(room, user, getRoomManager(), *this);
}

IRequestHandler* RequestHandlerFactory::createRoomMemberRequestHandler(Room room, LoggedUser user)
{
    return new RoomMemberRequestHandler(room, user, getRoomManager(), *this);
}

IRequestHandler* RequestHandlerFactory::createGameRequestHandler(LoggedUser user, int roomId)
{
    return new GameRequestHandler(user, roomId, GameManager::getInstance(m_database), *this);
}

LoginManager& RequestHandlerFactory::getLoginManager()
{
    return LoginManager::getInstance();
}

StatisticsManager& RequestHandlerFactory::getStatisticsManager()
{
    return StatisticsManager::getInstance(m_database);
}

RoomManager& RequestHandlerFactory::getRoomManager()
{
    return RoomManager::getInstance();
}

IDatabase& RequestHandlerFactory::getDatabase()
{
    return *m_database;
}

GameManager& RequestHandlerFactory::getGameManager()
{
    return GameManager::getInstance(m_database);
}