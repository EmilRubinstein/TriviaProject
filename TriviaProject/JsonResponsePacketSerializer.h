#pragma once
#include <iostream>
#include <map>
#include "Helper.h"
#include "GameManager.h"   // for PlayerResults

// structs v1.x.x
struct ErrorResponse { string message; };
struct LoginResponse { unsigned int status = 0; };
struct SignupResponse { unsigned int status = 0; };

// structs for v2.x.x
struct LogoutResponse { unsigned int status = 1; };
struct JoinRoomResponse { unsigned int status = 1; };
struct CreateRoomResponse { unsigned int status = 1; int roomId = 0; };
struct GetRoomsResponse
{
    std::vector<int> roomIds;
    std::vector<std::string> roomNames;
};
struct GetPlayersInRoomResponse { vector<string> players; };
struct LeaveRoomResponse { unsigned int status = 1; };
struct CloseRoomResponse { unsigned int status = 1; };
struct HighScoreResponse
{
    vector<string> userStatistics;
    vector<string> highScores;
};

// structs for v3.x.x
struct StartGameResponse { unsigned int status = 1; };
struct GetRoomStateResponse
{
    unsigned int status = 0;
    bool hasGameBegun = false;
    vector<string> players;
    unsigned int questionCount = 0;
    unsigned int answerTimeout = 0;
};

// structs for v4.0.0
struct LeaveGameResponse { unsigned int status = 1; };
struct SubmitAnswerResponse { unsigned int status = 1; };
struct GetQuestionResponse
{
    unsigned int status = 0;
    std::string question;
    std::map<int, std::string> answers;
};
struct GetGameResultsResponse
{
    unsigned int status = 0;
    std::vector<PlayerResults> results;
};

class JsonResponsePacketSerializer
{
public:
    static JsonResponsePacketSerializer& getInstance() {
        static JsonResponsePacketSerializer instance;
        return instance;
    }
    JsonResponsePacketSerializer(const JsonResponsePacketSerializer&) = delete;
    JsonResponsePacketSerializer& operator=(const JsonResponsePacketSerializer&) = delete;

    // v1.0.0
    static Buffer serializeResponse(const ErrorResponse& r);
    static Buffer serializeResponse(const LoginResponse& r);
    static Buffer serializeResponse(const SignupResponse& r);

    // v2.0.0
    static Buffer serializeLogoutResponse(const LogoutResponse& r);
    static Buffer serializeGetRoomsResponse(const GetRoomsResponse& r);
    static Buffer serializeGetPlayersInRoomResponse(const GetPlayersInRoomResponse& r);
    static Buffer serializeJoinRoomResponse(const JoinRoomResponse& r);
    static Buffer serializeCreateRoomResponse(const CreateRoomResponse& r);
    static Buffer serializeHighScoreResponse(const HighScoreResponse& r);

    // v3.0.0
    static Buffer serializeLeaveRoomResponse(const LeaveRoomResponse& r);
    static Buffer serializeCloseRoomResponse(const CloseRoomResponse& r);
    static Buffer serializeStartGameResponse(const StartGameResponse& r);
    static Buffer serializeGetRoomStateResponse(const GetRoomStateResponse& r);

    // v4.0.0
    static Buffer serializeLeaveGameResponse(const LeaveGameResponse& r);
    static Buffer serializeSubmitAnswerResponse(const SubmitAnswerResponse& r);
    static Buffer serializeGetQuestionResponse(const GetQuestionResponse& r);
    static Buffer serializeGetGameResultsResponse(const GetGameResultsResponse& r);

private:
    static Buffer buildBuffer(char code, const json& j);
    JsonResponsePacketSerializer() = default;
};