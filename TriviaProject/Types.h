#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <ctime>
#include "json.hpp"

using std::string;
using std::vector;
using json = nlohmann::json;
using Buffer = vector<unsigned char>;

class IRequestHandler;

struct RequestInfo
{
    char code = 0;
    time_t time = 0;
    Buffer json_raw_data;
};

struct RequestResult
{
    Buffer response;
    IRequestHandler* newHandler = nullptr;
};

enum CODES
{
    ERROR_CODE = 0,
    LOGIN,
    SIGN_UP,
    SIGNOUT_REQUEST,
    GET_ROOMS_REQUEST,
    GET_PLAYERS_IN_ROOM_REQUEST,
    GET_STATISTICS_REQUEST,
    GET_HIGH_SCORE_REQUEST,
    JOIN_ROOM_REQUEST,
    CREATE_ROOM_REQUEST,
    LEAVE_ROOM_REQUEST,
    CLOSE_ROOM_REQUEST,
    START_GAME_REQUEST,
    GET_ROOM_STATE_REQUEST,
    LEAVE_GAME_REQUEST,
    GET_QUESTION_REQUEST,
    SUBMIT_ANSWER_REQUEST,
    GET_GAME_RESULT_REQUEST,
};

void printRequest(const RequestInfo& req);
void printResponse(const RequestResult& res);
int readFully(SOCKET s, char* buffer, int len);