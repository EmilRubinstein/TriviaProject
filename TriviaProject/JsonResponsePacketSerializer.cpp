#include "JsonResponsePacketSerializer.h"

Buffer JsonResponsePacketSerializer::buildBuffer(char code, const json& j)
{
    std::string jsonStr = j.dump();
    Buffer buf;

    // 1 byte: response code
    buf.push_back(code);

    // 4 bytes: JSON size (little-endian)
    int size = (int)jsonStr.size();
    char sizeBytes[4];
    std::memcpy(sizeBytes, &size, 4);
    buf.insert(buf.end(), sizeBytes, sizeBytes + 4);

    // JSON data
    buf.insert(buf.end(), jsonStr.begin(), jsonStr.end());

    return buf;
}

// ─── v1.0.0 ──────────────────────────────────────────────────────────────────

Buffer JsonResponsePacketSerializer::serializeResponse(const ErrorResponse& r)
{
    json j = { {"message", r.message} };
    return buildBuffer(ERROR_CODE, j);
}

Buffer JsonResponsePacketSerializer::serializeResponse(const LoginResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(LOGIN, j);
}

Buffer JsonResponsePacketSerializer::serializeResponse(const SignupResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(SIGN_UP, j);
}

// ─── v2.0.0 ──────────────────────────────────────────────────────────────────

Buffer JsonResponsePacketSerializer::serializeLogoutResponse(const LogoutResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(SIGNOUT_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeGetRoomsResponse(const GetRoomsResponse& r)
{
    json j = {
        {"roomIds",   r.roomIds},
        {"roomNames", r.roomNames}
    };
    return buildBuffer(GET_ROOMS_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeGetPlayersInRoomResponse(const GetPlayersInRoomResponse& r)
{
    json j = { {"PlayersInRoom", r.players} };
    return buildBuffer(GET_PLAYERS_IN_ROOM_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeJoinRoomResponse(const JoinRoomResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(JOIN_ROOM_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeCreateRoomResponse(const CreateRoomResponse& r)
{
    json j = { {"status", r.status}, {"roomId", r.roomId} };
    return buildBuffer(CREATE_ROOM_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeHighScoreResponse(const HighScoreResponse& r)
{
    json j = {
        {"UserStatistics", r.userStatistics},
        {"HighScores",     r.highScores}
    };
    return buildBuffer(GET_HIGH_SCORE_REQUEST, j);
}

// ─── v3.0.0 ──────────────────────────────────────────────────────────────────

Buffer JsonResponsePacketSerializer::serializeLeaveRoomResponse(const LeaveRoomResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(LEAVE_ROOM_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeCloseRoomResponse(const CloseRoomResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(CLOSE_ROOM_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeStartGameResponse(const StartGameResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(START_GAME_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeGetRoomStateResponse(const GetRoomStateResponse& r)
{
    json j = {
        {"status",        r.status},
        {"hasGameBegun",  r.hasGameBegun},
        {"players",       r.players},
        {"questionCount", r.questionCount},
        {"answerTimeout", r.answerTimeout}
    };
    return buildBuffer(GET_ROOM_STATE_REQUEST, j);
}

// ─── v4.0.0 ──────────────────────────────────────────────────────────────────

Buffer JsonResponsePacketSerializer::serializeLeaveGameResponse(const LeaveGameResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(LEAVE_GAME_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeSubmitAnswerResponse(const SubmitAnswerResponse& r)
{
    json j = { {"status", r.status} };
    return buildBuffer(SUBMIT_ANSWER_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeGetQuestionResponse(const GetQuestionResponse& r)
{
    // Build answers as a JSON object with string keys ("1", "2", ...)
    json answersJson = json::object();
    for (const auto& [num, text] : r.answers)
        answersJson[std::to_string(num)] = text;

    json j = {
        {"status",   r.status},
        {"question", r.question},
        {"answers",  answersJson}
    };
    return buildBuffer(GET_QUESTION_REQUEST, j);
}

Buffer JsonResponsePacketSerializer::serializeGetGameResultsResponse(const GetGameResultsResponse& r)
{
    json resultsArr = json::array();
    for (const PlayerResults& pr : r.results)
    {
        resultsArr.push_back({
            {"username",            pr.username},
            {"correctAnswersCount", pr.correctAnswersCount},
            {"wrongAnswerCount",    pr.wrongAnswerCount},
            {"averageAnswerTime",   pr.averageAnswerTime}
            });
    }

    json j = {
        {"status",  r.status},
        {"results", resultsArr}
    };
    return buildBuffer(GET_GAME_RESULT_REQUEST, j);
}