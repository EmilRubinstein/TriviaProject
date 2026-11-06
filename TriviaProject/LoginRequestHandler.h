#pragma once
#include "IRequestHandler.h"
#include "LoginManager.h"

class RequestHandlerFactory; 

class LoginRequestHandler : public IRequestHandler {
public:
    LoginRequestHandler(LoginManager& loginManager, RequestHandlerFactory& factory);
    bool isRequestRelevant(RequestInfo req) override;
    RequestResult handleRequest(RequestInfo req) override;
private:
    RequestResult login(const RequestInfo& req);
    RequestResult signup(const RequestInfo& req);
    LoginManager& m_loginManager;
    RequestHandlerFactory& m_handlerFactory; 
};