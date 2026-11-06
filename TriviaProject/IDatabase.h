#pragma once
#include <string>
#include "Question.h"
#include "Room.h"

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual bool doesUserExist(const std::string& username) = 0;
    virtual bool doesPasswordMatch(const std::string& username, const std::string& password) = 0;
    virtual void addNewUser(const std::string& username, const std::string& password, const std::string& email) = 0;

    virtual int createRoom(const RoomData& roomData, const LoggedUser& user) = 0;

    virtual void deleteRoom(int roomId) = 0;

    // Questions
    virtual std::vector<Question> getQuestions(int amount) = 0;

    // Statistics
    virtual float getPlayerAverageAnswerTime(const std::string& username) = 0;
    virtual int getNumOfCorrectAnswers(const std::string& username) = 0;
    virtual int getNumOfTotalAnswers(const std::string& username) = 0;
    virtual int getNumOfPlayerGames(const std::string& username) = 0;
    virtual int getPlayerScore(const std::string& username) = 0;
    virtual void submitGameStatistics(const std::string& username, int correctAnswers, int wrongAnswers, float avgAnswerTime, int roomId) = 0;
    virtual std::vector<std::string> getHighScores() = 0;
};