#include "LoginRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "MenuRequestHandler.h" 
#include "JsonRequestPacketDeserializer.h"
#include "JsonResponsePacketSerializer.h"
#include <iostream>

LoginRequestHandler::LoginRequestHandler(LoginManager& loginManager, RequestHandlerFactory& factory)
    : m_loginManager(loginManager), m_handlerFactory(factory) {
}

bool LoginRequestHandler::isRequestRelevant(RequestInfo req) {
    return req.code == LOGIN || req.code == SIGN_UP;
}

RequestResult LoginRequestHandler::handleRequest(RequestInfo req) {
    if (req.code == LOGIN)
        return login(req);
    if (req.code == SIGN_UP)
        return signup(req);
    ErrorResponse err{ "Unknown request code" };
    return { JsonResponsePacketSerializer::serializeResponse(err), nullptr };
}

RequestResult LoginRequestHandler::login(const RequestInfo& req) {
    try
    {
        LoginRequest loginReq = JsonRequestPacketDeserializer::deserializeLoginRequest(req.json_raw_data);
        bool ok = m_loginManager.login(loginReq.username, loginReq.password);
        LoginResponse resp{ ok ? 1u : 0u };

        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);

        if (ok)
        {
            LoggedUser user(loginReq.username);
            result.newHandler = m_handlerFactory.createMenuRequestHandler(user);
        }
        else
        {
            result.newHandler = this;
        }
        return result;
    }
    catch (...)
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ "Login error" });
        result.newHandler = nullptr;
        return result;
    }
}

RequestResult LoginRequestHandler::signup(const RequestInfo& req) {
    try
    {
        SignupRequest signupReq = JsonRequestPacketDeserializer::deserializeSignupRequest(req.json_raw_data);
        bool ok = m_loginManager.signup(signupReq.username, signupReq.password, signupReq.email);
        SignupResponse resp{ ok ? 1u : 0u };

        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(resp);

        if (ok)
        {
            LoggedUser user(signupReq.username);
            result.newHandler = m_handlerFactory.createMenuRequestHandler(user);
        }
        else
        {
            result.newHandler = this;
        }
        return result;
    }
    catch (...)
    {
        RequestResult result;
        result.response = JsonResponsePacketSerializer::serializeResponse(ErrorResponse{ "Signup error" });
        result.newHandler = nullptr;
        return result;
    }
}