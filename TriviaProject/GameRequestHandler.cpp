#include "GameRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "GameManager.h"
#include "Room.h"
#include "JsonResponsePacketSerializer.h"
#include "JsonRequestPacketDeserializer.h"
#include "MenuRequestHandler.h"
#include <iostream>

GameRequestHandler::GameRequestHandler(LoggedUser user, int roomId,
    GameManager& gameManager,
    RequestHandlerFactory& handlerFactory)
    : m_user(user), m_roomId(roomId),
    m_gameManager(gameManager), m_handlerFactory(handlerFactory)
{}

bool GameRequestHandler::isRequestRelevant(RequestInfo requestInfo)
{
    return requestInfo.code == LEAVE_GAME_REQUEST ||
        requestInfo.code == GET_QUESTION_REQUEST ||
        requestInfo.code == SUBMIT_ANSWER_REQUEST ||
        requestInfo.code == GET_GAME_RESULT_REQUEST ||
        requestInfo.code == SIGNOUT_REQUEST;
}

RequestResult GameRequestHandler::handleRequest(RequestInfo requestInfo)
{
    RequestResult result;
    result.newHandler = nullptr;

    if (!isRequestRelevant(requestInfo))
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ "Request not relevant to game" });
        result.newHandler = this;
        return result;
    }

    switch (requestInfo.code)
    {
    case LEAVE_GAME_REQUEST:
        return leaveGame(requestInfo);
    case GET_QUESTION_REQUEST:
        return getQuestion(requestInfo);
    case SUBMIT_ANSWER_REQUEST:
        return submitAnswer(requestInfo);
    case GET_GAME_RESULT_REQUEST:
        return getGameResult(requestInfo);
    case SIGNOUT_REQUEST:
        return signOut(requestInfo);
    default:
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ "Unknown request" });
        result.newHandler = this;
        return result;
    }
}

RequestResult GameRequestHandler::leaveGame(RequestInfo requestInfo)
{
    RequestResult result;
    result.newHandler = nullptr;

    try
    {
        m_gameManager.removePlayer(m_user.getUsername());

        if (m_handlerFactory.getRoomManager().roomExists(m_roomId))
        {
            Room& room = m_handlerFactory.getRoomManager().getRoom(m_roomId);
            room.removeUser(m_user);
        }

        result.response = JsonResponsePacketSerializer::serializeLeaveGameResponse(LeaveGameResponse{});
        result.newHandler = m_handlerFactory.createMenuRequestHandler(m_user);
    }
    catch (const std::exception& e)
    {
        // Use e to avoid unreferenced warning
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult GameRequestHandler::getQuestion(RequestInfo requestInfo)
{
    RequestResult result;
    result.newHandler = nullptr;

    try
    {
        Question q;
        bool hasQuestion = m_gameManager.getQuestion(m_user.getUsername(), q);

        GetQuestionResponse response;
        response.status = 0;

        if (!hasQuestion)
        {
            response.status = 0;
            response.question = "";
            response.answers = {};
        }
        else
        {
            response.status = 1;
            response.question = q.getQuestion();

            const std::vector<std::string>& possibleAnswers = q.getPossibleAnswers();
            for (size_t i = 0; i < possibleAnswers.size(); ++i)
            {
                response.answers[static_cast<int>(i) + 1] = possibleAnswers[i];
            }
        }

        result.response = JsonResponsePacketSerializer::serializeGetQuestionResponse(response);
        result.newHandler = this;
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult GameRequestHandler::submitAnswer(RequestInfo requestInfo)
{
    RequestResult result;
    result.newHandler = nullptr;

    try
    {
        SubmitAnswerRequest req =
            JsonRequestPacketDeserializer::deserializeSubmitAnswerRequest(
                requestInfo.json_raw_data);

        m_gameManager.submitAnswer(m_user.getUsername(), req.answerId, req.answerTime);

        result.response = JsonResponsePacketSerializer::serializeSubmitAnswerResponse(SubmitAnswerResponse{});
        result.newHandler = this;
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult GameRequestHandler::getGameResult(RequestInfo requestInfo)
{
    RequestResult result;

    try
    {
        GetGameResultsResponse response;
        response.status = 0;

        if (m_gameManager.isGameFinished(m_roomId))
        {
            response.status = 1;
            response.results = m_gameManager.getGameResults(m_roomId);
        }

        result.response = JsonResponsePacketSerializer::serializeGetGameResultsResponse(response);

        if (response.status == 1)
        {
            if (m_handlerFactory.getRoomManager().roomExists(m_roomId))
                m_handlerFactory.getRoomManager().deleteRoom(m_roomId);
            result.newHandler = m_handlerFactory.createMenuRequestHandler(m_user);
        }
        else
            result.newHandler = this;
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}
RequestResult GameRequestHandler::signOut(RequestInfo requestInfo)
{
    RequestResult result;
    m_gameManager.removePlayer(m_user.getUsername());
    m_handlerFactory.getLoginManager().logout(m_user.getUsername());
    result.response = JsonResponsePacketSerializer::serializeLogoutResponse(LogoutResponse{});
    result.newHandler = m_handlerFactory.createLoginRequestHandler();
    return result;
}