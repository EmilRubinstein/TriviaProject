#pragma once
#include "IDatabase.h"
#include "LoggedUser.h"
#include <vector>
#include <string>

class LoginManager {
public:
    static LoginManager& getInstance(IDatabase* database = nullptr) {
        static LoginManager instance(database);
        return instance;
    }

    LoginManager(const LoginManager&) = delete;
    LoginManager& operator=(const LoginManager&) = delete;

    bool login(const std::string& username, const std::string& password);
    bool signup(const std::string& username, const std::string& password, const std::string& email);
    void logout(const std::string& username);

private:
    explicit LoginManager(IDatabase* database);
    bool isUserLoggedIn(const std::string& username) const;

    IDatabase* m_database;
    std::vector<LoggedUser> m_loggedUsers;
};