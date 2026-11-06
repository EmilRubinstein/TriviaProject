#pragma once
#include "IRequestHandler.h"
#include "LoggedUser.h"
#include "Room.h"
#include "RoomManager.h"

class RequestHandlerFactory;

class RoomMemberRequestHandler : public IRequestHandler
{
public:
    RoomMemberRequestHandler(Room room, LoggedUser user, RoomManager& roomManager, RequestHandlerFactory& handlerFactory);

    bool isRequestRelevant(RequestInfo requestInfo) override;
    RequestResult handleRequest(RequestInfo requestInfo) override;

private:
    Room m_room;
    LoggedUser m_user;
    RoomManager& m_roomManager;
    RequestHandlerFactory& m_handlerFactory;

    RequestResult leaveRoom(RequestInfo requestInfo);
    RequestResult getRoomState(RequestInfo requestInfo);
    RequestResult getPlayersInRoom(RequestInfo requestInfo);
};