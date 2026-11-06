#pragma once
#include "IDatabase.h"
#include "sqlite3.h"

class SqliteDataBase : public IDatabase {
public:
    static SqliteDataBase& getInstance() {
        static SqliteDataBase instance; 
        return instance;
    }

    void submitGameStatistics(const std::string& username, int correctAnswers, int wrongAnswers, float avgAnswerTime, int roomId) override;

    SqliteDataBase(const SqliteDataBase&) = delete;
    SqliteDataBase& operator=(const SqliteDataBase&) = delete;

    ~SqliteDataBase() override;

    bool doesUserExist(const std::string& username) override;
    bool doesPasswordMatch(const std::string& username, const std::string& password) override;
    void addNewUser(const std::string& username, const std::string& password, const std::string& email) override;

    int createRoom(const RoomData& roomData, const LoggedUser& user);

    std::vector<Question> getQuestions(int amount) override;
    float getPlayerAverageAnswerTime(const std::string& username) override;
    int getNumOfCorrectAnswers(const std::string& username) override;
    int getNumOfTotalAnswers(const std::string& username) override;
    int getNumOfPlayerGames(const std::string& username) override;
    int getPlayerScore(const std::string& username) override;
    std::vector<std::string> getHighScores() override;

    void deleteRoom(int roomId);
private:
    SqliteDataBase();
    void createTables();
    void initQuestionsTable();
    sqlite3* _db;
};