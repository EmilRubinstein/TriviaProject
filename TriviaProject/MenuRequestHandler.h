#pragma once

#include "IRequestHandler.h"
#include "LoggedUser.h"
#include "RequestHandlerFactory.h"

class MenuRequestHandler : public IRequestHandler
{
public:
    MenuRequestHandler(LoggedUser user, RequestHandlerFactory& handlerFactory);
    virtual bool isRequestRelevant(RequestInfo requestInfo) override;
    virtual RequestResult handleRequest(RequestInfo requestInfo) override;
    LoggedUser getUser() const;
private:
    RequestResult signout(RequestInfo requestInfo);
    RequestResult getRooms(RequestInfo requestInfo);
    RequestResult getPlayersInRoom(RequestInfo requestInfo);
    RequestResult getPersonalStats(RequestInfo requestInfo);
    RequestResult getHighScore(RequestInfo requestInfo);
    RequestResult joinRoom(RequestInfo requestInfo);
    RequestResult createRoom(RequestInfo requestInfo);
    RequestResult leaveRoom(RequestInfo requestInfo);
    RequestResult closeRoom(RequestInfo requestInfo);

    LoggedUser m_user;
    RequestHandlerFactory& m_handlerFactory;
};