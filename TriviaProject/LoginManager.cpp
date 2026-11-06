#include "LoginManager.h"
#include <algorithm>
#include <iostream>

LoginManager::LoginManager(IDatabase* database) : m_database(database) {}
// logged users are stored inside of _loggedUsers vector that's initialized in loginManger.h as empty cuz there are no active users at the start of the server
bool LoginManager::login(const std::string& username, const std::string& password) {
    if (!m_database->doesUserExist(username)) {
        std::cout << "[LoginManager] Login failed - user not found: " << username << std::endl;
        return false;
    }
    if (isUserLoggedIn(username)) {
        std::cout << "[LoginManager] Login failed - already logged in: " << username << std::endl;
        return false;
    }
    if (!m_database->doesPasswordMatch(username, password)) {
        std::cout << "[LoginManager] Login failed - wrong password: " << username << std::endl;
        return false;
    }
    m_loggedUsers.emplace_back(username);
    std::cout << "[LoginManager] User logged in: " << username << std::endl;
    return true;
}

bool LoginManager::signup(const std::string& username, const std::string& password, const std::string& email) {
    if (m_database == nullptr)
    {
        std::cerr << "[LoginManager] ERROR: database is null!" << std::endl;
        return false;
    }
    if (m_database->doesUserExist(username)) {
        std::cout << "[LoginManager] Signup failed - username taken: " << username << std::endl;
        return false;
    }
    m_database->addNewUser(username, password, email);
    std::cout << "[LoginManager] New user registered: " << username << std::endl;
    return true;
}

void LoginManager::logout(const std::string& username) {
    auto it = std::find_if(m_loggedUsers.begin(), m_loggedUsers.end(),
        [&](const LoggedUser& u) { return u.getUsername() == username; });
    if (it != m_loggedUsers.end()) {
        m_loggedUsers.erase(it);
        std::cout << "[LoginManager] User logged out: " << username << std::endl;
    }
}

bool LoginManager::isUserLoggedIn(const std::string& username) const {
    return std::any_of(m_loggedUsers.begin(), m_loggedUsers.end(),
        [&](const LoggedUser& u) { return u.getUsername() == username; });
}