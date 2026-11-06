#include "MenuRequestHandler.h"
#include "JsonRequestPacketDeserializer.h"
#include "JsonResponsePacketSerializer.h"
#include "LoginRequestHandler.h" 
#include "RoomAdminRequestHandler.h" 
#include "RoomMemberRequestHandler.h"

MenuRequestHandler::MenuRequestHandler(LoggedUser user, RequestHandlerFactory& handlerFactory)
    : m_user(user), m_handlerFactory(handlerFactory)
{
}

LoggedUser MenuRequestHandler::getUser() const
{
    return m_user;
}

bool MenuRequestHandler::isRequestRelevant(RequestInfo requestInfo)
{
    return  requestInfo.code == SIGNOUT_REQUEST ||
        requestInfo.code == GET_ROOMS_REQUEST ||
        requestInfo.code == GET_PLAYERS_IN_ROOM_REQUEST ||
        requestInfo.code == GET_STATISTICS_REQUEST ||
        requestInfo.code == GET_HIGH_SCORE_REQUEST ||
        requestInfo.code == JOIN_ROOM_REQUEST ||
        requestInfo.code == CREATE_ROOM_REQUEST ||
        requestInfo.code == LEAVE_ROOM_REQUEST ||
		requestInfo.code == CLOSE_ROOM_REQUEST;
}

RequestResult MenuRequestHandler::handleRequest(RequestInfo requestInfo)
{
    if (!isRequestRelevant(requestInfo))
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ "Request not relevant to menu" });
        result.newHandler = this;
        return result;
    }

    switch (requestInfo.code)
    {
    case SIGNOUT_REQUEST:             return signout(requestInfo);
    case GET_ROOMS_REQUEST:           return getRooms(requestInfo);
    case GET_PLAYERS_IN_ROOM_REQUEST: return getPlayersInRoom(requestInfo);
    case GET_STATISTICS_REQUEST:      return getPersonalStats(requestInfo);
    case GET_HIGH_SCORE_REQUEST:      return getHighScore(requestInfo);
    case JOIN_ROOM_REQUEST:           return joinRoom(requestInfo);
    case CREATE_ROOM_REQUEST:         return createRoom(requestInfo);
    case LEAVE_ROOM_REQUEST:          return leaveRoom(requestInfo);
    case CLOSE_ROOM_REQUEST:          return closeRoom(requestInfo);
    default:
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ "Unknown request" });
        result.newHandler = this;
        return result;
    }
    }
}

RequestResult MenuRequestHandler::signout(RequestInfo requestInfo)
{
    RequestResult result;
    m_handlerFactory.getLoginManager().logout(m_user.getUsername());
    result.response = JsonResponsePacketSerializer::serializeLogoutResponse(LogoutResponse{});
    result.newHandler = m_handlerFactory.createLoginRequestHandler();
    return result;
}

RequestResult MenuRequestHandler::getRooms(RequestInfo requestInfo)
{
    RequestResult result;

    std::vector<int> roomIds = m_handlerFactory.getRoomManager().getRooms();

    std::vector<int> ids;
    std::vector<std::string> names;

    for (int id : roomIds)
    {
        Room& room = m_handlerFactory.getRoomManager().getRoom(id);
        ids.push_back(id);
        names.push_back(room.getRoomData().name);
    }

    GetRoomsResponse response;
    response.roomIds = ids;
    response.roomNames = names;

    result.response = JsonResponsePacketSerializer::serializeGetRoomsResponse(response);
    result.newHandler = this;
    return result;
}

RequestResult MenuRequestHandler::getPlayersInRoom(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        GetPlayersInRoomRequest req = JsonRequestPacketDeserializer::deserializeGetPlayersInRoomRequest(requestInfo.json_raw_data);

        if (!m_handlerFactory.getRoomManager().roomExists(req.roomId))
        {
            // Room doesn't exist - return empty list
            GetPlayersInRoomResponse response;
            response.players = std::vector<std::string>();
            result.response = JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(response);
            result.newHandler = this;
            return result;
        }

        Room& room = m_handlerFactory.getRoomManager().getRoom(req.roomId);

        // Get current players
        std::vector<std::string> players = room.getAllUsers();

        // Log for debugging
        std::cout << "[Server] Room " << req.roomId << " has " << players.size() << " players: ";
        for (const auto& p : players) std::cout << p << " ";
        std::cout << std::endl;

        result.response = JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(GetPlayersInRoomResponse{ players });
    }
    catch (const std::exception& e)
    {
        GetPlayersInRoomResponse response;
        response.players = std::vector<std::string>();
        result.response = JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(response);
    }
    result.newHandler = this;
    return result;
}

RequestResult MenuRequestHandler::getPersonalStats(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        vector<string> stats = m_handlerFactory.getStatisticsManager().getUserStatistics(m_user.getUsername());
        result.response = JsonResponsePacketSerializer::serializeHighScoreResponse(
            HighScoreResponse{ stats, {} }
        );
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ e.what() }
        );
    }
    result.newHandler = this;
    return result;
}

RequestResult MenuRequestHandler::getHighScore(RequestInfo requestInfo)
{
    RequestResult result;
    try
    {
        vector<string> scores = m_handlerFactory.getStatisticsManager().getHighScore();
        result.response = JsonResponsePacketSerializer::serializeHighScoreResponse(
            HighScoreResponse{ {}, scores }
        );
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ e.what() }
        );
    }
    result.newHandler = this;
    return result;
}

RequestResult MenuRequestHandler::joinRoom(RequestInfo requestInfo)
{
    RequestResult result;
    JoinRoomRequest req = JsonRequestPacketDeserializer::deserializeJoinRoomRequest(requestInfo.json_raw_data);

    // add user to the room
    try
    {
        Room& room = m_handlerFactory.getRoomManager().getRoom(req.roomId);
        room.addUser(m_user);
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(
            ErrorResponse{ e.what() }
        );
    }

    result.response = JsonResponsePacketSerializer::serializeJoinRoomResponse(JoinRoomResponse{});
    Room& joinedRoom = m_handlerFactory.getRoomManager().getRoom(req.roomId);
    result.newHandler = m_handlerFactory.createRoomMemberRequestHandler(joinedRoom, m_user);
    return result;
}
RequestResult MenuRequestHandler::createRoom(RequestInfo requestInfo)
{
    RequestResult result;

    CreateRoomRequest req =
        JsonRequestPacketDeserializer::deserializeCreateRoomRequest(
            requestInfo.json_raw_data);

    RoomData data;
    data.name = req.roomName;
    data.creator = m_user.getUsername();
    data.maxPlayers = req.maxPlayers;
    data.timePerQuestion = req.timePerQuestion;
    data.numOfQuestionsInGame = 10;
    data.status = WAITING;

    // create room in DB FIRST and get ID
    int roomId = m_handlerFactory.getDatabase().createRoom(data, m_user);

    data.id = roomId;

    // store in memory manager
    m_handlerFactory.getRoomManager().createRoom(m_user, data);

    // Create response with roomId
    CreateRoomResponse response;
    response.status = 1;
    response.roomId = roomId;

    result.response = JsonResponsePacketSerializer::serializeCreateRoomResponse(response);
    Room& newRoom = m_handlerFactory.getRoomManager().getRoom(roomId);
    result.newHandler = m_handlerFactory.createRoomAdminRequestHandler(newRoom, m_user);

    return result;
}

RequestResult MenuRequestHandler::leaveRoom(RequestInfo requestInfo)
{
    RequestResult result;

    try
    {
        LeaveRoomRequest req = JsonRequestPacketDeserializer::deserializeLeaveRoomRequest(requestInfo.json_raw_data);

        if (!m_handlerFactory.getRoomManager().roomExists(req.roomId))
        {
            result.response = JsonResponsePacketSerializer::serializeLeaveRoomResponse(LeaveRoomResponse{});
            result.newHandler = this;
            return result;
        }

        Room& room = m_handlerFactory.getRoomManager().getRoom(req.roomId);
        RoomData roomData = room.getRoomData();

        // Check if the person leaving is the admin
        bool isAdmin = (roomData.creator == m_user.getUsername());

        // Remove the user from the room
        room.removeUser(m_user);
        std::cout << "[Server] User " << m_user.getUsername() << " left room " << req.roomId << std::endl;

        // If admin left OR room is empty, delete the room
        if (isAdmin || room.getAllUsers().empty())
        {
            std::cout << "[Server] Admin left or room empty - deleting room " << req.roomId << std::endl;
            m_handlerFactory.getRoomManager().deleteRoom(req.roomId);
        }

        result.response = JsonResponsePacketSerializer::serializeLeaveRoomResponse(LeaveRoomResponse{});
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
    }

    result.newHandler = this;
    return result;
}

RequestResult MenuRequestHandler::closeRoom(RequestInfo requestInfo)
{
    RequestResult result;

    try
    {
        CloseRoomRequest req = JsonRequestPacketDeserializer::deserializeCloseRoomRequest(requestInfo.json_raw_data);

        if (!m_handlerFactory.getRoomManager().roomExists(req.roomId))
        {
            result.response = JsonResponsePacketSerializer::serializeCloseRoomResponse(CloseRoomResponse{});
            result.newHandler = this;
            return result;
        }

        Room& room = m_handlerFactory.getRoomManager().getRoom(req.roomId);
        RoomData roomData = room.getRoomData();

        // Check if user is the admin (creator)
        if (roomData.creator != m_user.getUsername())
        {
            result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ "Only the room creator can close the room" });
            result.newHandler = this;
            return result;
        }

        // Get all players in the room
        std::vector<std::string> players = room.getAllUsers();

        std::cout << "[Server] Admin " << m_user.getUsername() << " closing room " << req.roomId
            << " with " << players.size() << " players" << std::endl;

        // Remove ALL players from the room
        for (const std::string& playerName : players)
        {
            LoggedUser player(playerName);
            room.removeUser(player);
            std::cout << "[Server] Removed " << playerName << " from closed room " << req.roomId << std::endl;
        }
        

        // Delete the room
        m_handlerFactory.getRoomManager().deleteRoom(req.roomId);
        m_handlerFactory.getDatabase().deleteRoom(req.roomId);
        std::cout << "[Server] Room " << req.roomId << " deleted by admin" << std::endl;


      

        result.response = JsonResponsePacketSerializer::serializeCloseRoomResponse(CloseRoomResponse{});
    }
    catch (const std::exception& e)
    {
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ e.what() });
    }

    result.newHandler = this;
    return result;
}