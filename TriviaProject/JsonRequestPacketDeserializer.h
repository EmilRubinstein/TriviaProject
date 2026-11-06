#pragma once
#include "json.hpp"
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using json = nlohmann::json;
using Buffer = vector<unsigned char>;

// structs v1.0.0
struct LoginRequest { string username; string password; };
struct SignupRequest { string username; string password; string email; };

// structs v2.0.0
struct GetPlayersInRoomRequest { int roomId = 0; };
struct JoinRoomRequest { int roomId = 0; };
struct CreateRoomRequest
{
    string roomName;
    int maxPlayers = 4;
    int timePerQuestion = 30;
};

struct CloseRoomRequest { int roomId = 0; };
struct LeaveRoomRequest { int roomId = 0; };

// v4.0.0
struct SubmitAnswerRequest
{
    int answerId = 0;
    float answerTime = 0.0f;
};

class JsonRequestPacketDeserializer
{
public:
    static JsonRequestPacketDeserializer& getInstance() {
        static JsonRequestPacketDeserializer instance;
        return instance;
    }
    JsonRequestPacketDeserializer(const JsonRequestPacketDeserializer&) = delete;
    JsonRequestPacketDeserializer& operator=(const JsonRequestPacketDeserializer&) = delete;

    // structs v1.0.0
    static LoginRequest  deserializeLoginRequest(Buffer b);
    static SignupRequest deserializeSignupRequest(Buffer b);

    // structs v2.0.0
    static GetPlayersInRoomRequest deserializeGetPlayersInRoomRequest(Buffer b);
    static JoinRoomRequest deserializeJoinRoomRequest(Buffer b);
    static CreateRoomRequest deserializeCreateRoomRequest(Buffer b);
    static LeaveRoomRequest deserializeLeaveRoomRequest(Buffer b);
    static CloseRoomRequest deserializeCloseRoomRequest(Buffer b);

    // v4.0.0
    static SubmitAnswerRequest deserializeSubmitAnswerRequest(Buffer b);

private:
    JsonRequestPacketDeserializer() = default;
};