#pragma once

#include <string>
#include <vector>
#include <map>
#include <ctime>

#include "IDatabase.h"
#include "Question.h"
#include "LoggedUser.h"

// Per-player state tracked during an active game
struct PlayerGameState
{
    int  currentQuestionIndex = 0;
    int  correctAnswers = 0;
    int  wrongAnswers = 0;
    float totalAnswerTime = 0.0f;
    int  answersSubmitted = 0;
    bool finished = false;
};

// Carries one player's final results
struct PlayerResults
{
    std::string username;
    int         correctAnswersCount = 0;
    int         wrongAnswerCount = 0;
    float       averageAnswerTime = 0.0f;
};

class GameManager
{
public:
    static GameManager& getInstance(IDatabase* database = nullptr)
    {
        static GameManager instance(database);
        return instance;
    }

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    void startGame(int roomId, const std::vector<std::string>& players,
        int numQuestions, int timePerQuestion);
    bool getQuestion(const std::string& username, Question& outQuestion);
    void submitAnswer(const std::string& username, int answerId, float answerTime);
    bool isGameFinished(int roomId) const;
    std::vector<PlayerResults> getGameResults(int roomId);
    void removePlayer(const std::string& username);
    bool gameExists(int roomId) const;

private:
    explicit GameManager(IDatabase* database);
    void persistResults(int roomId);

    IDatabase* m_database;
    std::map<int, std::vector<Question>> m_roomQuestions;
    std::map<int, std::vector<std::string>> m_roomPlayers;
    std::map<int, int> m_roomTimePerQuestion;
    std::map<std::string, PlayerGameState> m_playerStates;
    std::map<std::string, int> m_playerRoom;
};