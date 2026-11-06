#include "JsonRequestPacketDeserializer.h"

// readRequest already strips the header and puts ONLY the jsons body
// into json_raw_data so we parse it directly, no offset needed
static json extractJson(const Buffer& b)
{
    std::string jsonStr(b.begin(), b.end());
    return json::parse(jsonStr);
}

LoginRequest JsonRequestPacketDeserializer::deserializeLoginRequest(Buffer b)
{
    json j = extractJson(b);

    LoginRequest req;
    j.at("username").get_to(req.username);
    j.at("password").get_to(req.password);

    return req;
}

SignupRequest JsonRequestPacketDeserializer::deserializeSignupRequest(Buffer b)
{
    json j = extractJson(b);

    SignupRequest req;
    j.at("username").get_to(req.username);
    j.at("password").get_to(req.password);
    j.at("email").get_to(req.email);

    return req;
}

//v2.0.0   

GetPlayersInRoomRequest JsonRequestPacketDeserializer::deserializeGetPlayersInRoomRequest(Buffer b)
{
    json j = extractJson(b);
    GetPlayersInRoomRequest req;
    j.at("roomId").get_to(req.roomId);
    return req;
}

JoinRoomRequest JsonRequestPacketDeserializer::deserializeJoinRoomRequest(Buffer b)
{
    json j = extractJson(b);
    JoinRoomRequest req;
    j.at("roomId").get_to(req.roomId);
    return req;
}

CreateRoomRequest JsonRequestPacketDeserializer::deserializeCreateRoomRequest(Buffer b)
{
    json j = extractJson(b);
    CreateRoomRequest req;
    j.at("roomName").get_to(req.roomName);
    j.at("maxPlayers").get_to(req.maxPlayers);
    j.at("timePerQuestion").get_to(req.timePerQuestion);
    return req;
}

LeaveRoomRequest JsonRequestPacketDeserializer::deserializeLeaveRoomRequest(Buffer b)
{
    json j = extractJson(b);
    LeaveRoomRequest req;
    j.at("roomId").get_to(req.roomId);
    return req;
}

CloseRoomRequest JsonRequestPacketDeserializer::deserializeCloseRoomRequest(Buffer b)
{
    json j = extractJson(b);
    CloseRoomRequest req;
    j.at("roomId").get_to(req.roomId);
    return req;
}
SubmitAnswerRequest JsonRequestPacketDeserializer::deserializeSubmitAnswerRequest(Buffer b)
{
    json j = extractJson(b);
    SubmitAnswerRequest req;
    j.at("answerId").get_to(req.answerId);
    j.at("answerTime").get_to(req.answerTime);
    return req;
}