#include "RoomAdminRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "JsonResponsePacketSerializer.h"
#include "MenuRequestHandler.h"
#include "GameRequestHandler.h"
#include <iostream>

RoomAdminRequestHandler::RoomAdminRequestHandler(Room room, LoggedUser user,
    RoomManager& roomManager,
    RequestHandlerFactory& handlerFactory)
    : m_room(room), m_user(user), m_roomManager(roomManager), m_handlerFactory(handlerFactory)
{
}

bool RoomAdminRequestHandler::isRequestRelevant(RequestInfo requestInfo)
{
    return requestInfo.code == CLOSE_ROOM_REQUEST ||
        requestInfo.code == START_GAME_REQUEST ||
        requestInfo.code == GET_ROOM_STATE_REQUEST||
        requestInfo.code == GET_PLAYERS_IN_ROOM_REQUEST;
}

RequestResult RoomAdminRequestHandler::handleRequest(RequestInfo requestInfo)
{
    if (!isRequestRelevant(requestInfo))
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ "Request not relevant to room admin" });
        result.newHandler = this;
        return result;
    }

    switch (requestInfo.code)
    {
    case CLOSE_ROOM_REQUEST:      return closeRoom(requestInfo);
    case START_GAME_REQUEST:      return startGame(requestInfo);
    case GET_ROOM_STATE_REQUEST:  return getRoomState(requestInfo);
    case GET_PLAYERS_IN_ROOM_REQUEST: return getPlayersInRoom(requestInfo);
    default:
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ "Unknown request" });
        result.newHandler = this;
        return result;
    }
    }
}

RequestResult RoomAdminRequestHandler::closeRoom(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        int roomId = m_room.getRoomData().id;

        if (m_roomManager.roomExists(roomId))
        {
            Room& liveRoom = m_roomManager.getRoom(roomId);

            // Evict every player
            std::vector<std::string> players = liveRoom.getAllUsers();
            for (const std::string& playerName : players)
            {
                LoggedUser player(playerName);
                liveRoom.removeUser(player);
                std::cout << "[RoomAdmin] Removed " << playerName
                    << " from room " << roomId << std::endl;
            }

            m_roomManager.deleteRoom(roomId);
            m_handlerFactory.getDatabase().deleteRoom(roomId);

            std::cout << "[RoomAdmin] Room " << roomId
                << " closed by admin " << m_user.getUsername() << std::endl;
        }

        result.response = JsonResponsePacketSerializer::serializeCloseRoomResponse(CloseRoomResponse{});
        result.newHandler = m_handlerFactory.createMenuRequestHandler(m_user);
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult RoomAdminRequestHandler::startGame(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        int roomId = m_room.getRoomData().id;

        if (!m_roomManager.roomExists(roomId))
        {
            result.response = JsonResponsePacketSerializer::serializeResponse(
                ErrorResponse{ "Room not found" });
            result.newHandler = this;
            return result;
        }

        Room& liveRoom = m_roomManager.getRoom(roomId);
        RoomData roomData = liveRoom.getRoomData();

        std::vector<std::string> players = liveRoom.getAllUsers();
        m_handlerFactory.getGameManager().startGame(
            roomId,
            players,
            roomData.numOfQuestionsInGame,
            roomData.timePerQuestion
        );

        liveRoom.setStatus(ACTIVE);
        m_room.setStatus(ACTIVE);

        std::cout << "[RoomAdmin] Game started in room " << roomId << std::endl;

        result.response = JsonResponsePacketSerializer::serializeStartGameResponse(StartGameResponse{});
        result.newHandler = this;
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult RoomAdminRequestHandler::getRoomState(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        int roomId = m_room.getRoomData().id;

        if (!m_roomManager.roomExists(roomId))
        {
            GetRoomStateResponse response;
            response.status = 0;
            response.hasGameBegun = false;
            response.players = {};
            response.questionCount = 0;
            response.answerTimeout = 0;

            result.response = JsonResponsePacketSerializer::serializeGetRoomStateResponse(response);
            result.newHandler = m_handlerFactory.createMenuRequestHandler(m_user);
            return result;
        }

        Room& liveRoom = m_roomManager.getRoom(roomId);
        RoomData roomData = liveRoom.getRoomData();

        GetRoomStateResponse response;
        response.status = 1;
        response.hasGameBegun = (roomData.status == ACTIVE);
        response.players = liveRoom.getAllUsers();
        response.questionCount = roomData.numOfQuestionsInGame;
        response.answerTimeout = roomData.timePerQuestion;

        result.response = JsonResponsePacketSerializer::serializeGetRoomStateResponse(response);

        if (roomData.status == ACTIVE)
        {
            result.newHandler = m_handlerFactory.createGameRequestHandler(m_user, roomId);
            std::cout << "[RoomAdminRequestHandler] Game started - transitioning to GameRequestHandler for " << m_user.getUsername() << std::endl;
        }
        else
        {
            result.newHandler = this;
        }
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult RoomAdminRequestHandler::getPlayersInRoom(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        int roomId = m_room.getRoomData().id;

        if (!m_roomManager.roomExists(roomId))
        {
            GetPlayersInRoomResponse response;
            response.players = {};
            result.response = JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(response);
            result.newHandler = this;
            return result;
        }

        Room& liveRoom = m_roomManager.getRoom(roomId);
        std::vector<std::string> players = liveRoom.getAllUsers();

        result.response = JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(
            GetPlayersInRoomResponse{ players });
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
    }
    result.newHandler = this;
    return result;
}