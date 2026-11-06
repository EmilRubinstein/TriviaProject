#pragma once

#include "IRequestHandler.h"
#include "LoggedUser.h"

class GameManager;
class RequestHandlerFactory;
class Room;

class GameRequestHandler : public IRequestHandler
{
public:
    GameRequestHandler(LoggedUser user, int roomId,
        GameManager& gameManager,
        RequestHandlerFactory& handlerFactory);

    bool isRequestRelevant(RequestInfo requestInfo) override;
    RequestResult handleRequest(RequestInfo requestInfo) override;

private:
    RequestResult leaveGame(RequestInfo requestInfo);
    RequestResult getQuestion(RequestInfo requestInfo);
    RequestResult submitAnswer(RequestInfo requestInfo);
    RequestResult getGameResult(RequestInfo requestInfo);
    RequestResult signOut(RequestInfo requestInfo);

    LoggedUser m_user;
    int m_roomId;
    GameManager& m_gameManager;
    RequestHandlerFactory& m_handlerFactory;
};