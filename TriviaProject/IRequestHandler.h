#pragma once
#include "Types.h"

#include <WinSock2.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <sstream>

class IRequestHandler
{
public:
    virtual ~IRequestHandler() = default;

    virtual bool isRequestRelevant(RequestInfo req) = 0;
    virtual RequestResult handleRequest(RequestInfo req) = 0;
};