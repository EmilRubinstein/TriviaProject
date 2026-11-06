#include "RoomMemberRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "JsonResponsePacketSerializer.h"
#include "MenuRequestHandler.h"
#include <iostream>

RoomMemberRequestHandler::RoomMemberRequestHandler(Room room, LoggedUser user,
    RoomManager& roomManager,
    RequestHandlerFactory& handlerFactory)
    : m_room(room), m_user(user), m_roomManager(roomManager), m_handlerFactory(handlerFactory)
{
}

bool RoomMemberRequestHandler::isRequestRelevant(RequestInfo requestInfo)
{
    return requestInfo.code == LEAVE_ROOM_REQUEST ||
        requestInfo.code == GET_ROOM_STATE_REQUEST||
        requestInfo.code == GET_PLAYERS_IN_ROOM_REQUEST;
}

RequestResult RoomMemberRequestHandler::handleRequest(RequestInfo requestInfo)
{
    if (!isRequestRelevant(requestInfo))
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ "Request not relevant to room member" });
        result.newHandler = this;
        return result;
    }

    switch (requestInfo.code)
    {
    case LEAVE_ROOM_REQUEST:      return leaveRoom(requestInfo);
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

RequestResult RoomMemberRequestHandler::leaveRoom(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        int roomId = m_room.getRoomData().id;

        if (m_roomManager.roomExists(roomId))
        {
            Room& liveRoom = m_roomManager.getRoom(roomId);
            liveRoom.removeUser(m_user);

            std::cout << "[RoomMember] " << m_user.getUsername()
                << " left room " << roomId << std::endl;

            if (liveRoom.getAllUsers().empty())
            {
                m_roomManager.deleteRoom(roomId);
                m_handlerFactory.getDatabase().deleteRoom(roomId);
                std::cout << "[RoomMember] Room " << roomId
                    << " deleted (now empty)" << std::endl;
            }
        }

        result.response = JsonResponsePacketSerializer::serializeLeaveRoomResponse(LeaveRoomResponse{});
        result.newHandler = m_handlerFactory.createMenuRequestHandler(m_user);
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
        result.newHandler = this;
    }
    return result;
}

RequestResult RoomMemberRequestHandler::getRoomState(RequestInfo requestInfo)
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
            std::cout << "[RoomMemberRequestHandler] Game started - transitioning to GameRequestHandler for " << m_user.getUsername() << std::endl;
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

RequestResult RoomMemberRequestHandler::getPlayersInRoom(RequestInfo requestInfo)
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