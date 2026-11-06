#include "GameManager.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

GameManager::GameManager(IDatabase* database)
    : m_database(database)
{}

// ─────────────────────────────────────────────────────────────────────────────
// startGame
// Called by RoomAdminRequestHandler when START_GAME_REQUEST is received.
// Loads questions from the DB and creates a blank PlayerGameState for every player.
// ─────────────────────────────────────────────────────────────────────────────
void GameManager::startGame(int roomId,
    const std::vector<std::string>& players,
    int numQuestions,
    int timePerQuestion)
{
    // Clean up any stale state for this room before starting fresh
    if (m_roomQuestions.count(roomId))
    {
        for (const std::string& p : m_roomPlayers[roomId])
        {
            m_playerStates.erase(p);
            m_playerRoom.erase(p);
        }
        m_roomQuestions.erase(roomId);
        m_roomPlayers.erase(roomId);
        m_roomTimePerQuestion.erase(roomId);
    }

    // Load random questions from DB
    std::vector<Question> questions = m_database->getQuestions(numQuestions);

    m_roomQuestions[roomId] = questions;
    m_roomPlayers[roomId] = players;
    m_roomTimePerQuestion[roomId] = timePerQuestion;

    for (const std::string& username : players)
    {
        m_playerStates[username] = PlayerGameState{};
        m_playerRoom[username] = roomId;
    }

    std::cout << "[GameManager] Game started in room " << roomId
        << " with " << players.size() << " players and "
        << questions.size() << " questions." << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// getQuestion
// Retrieves the current question for this player.
// Returns false when the player has answered all questions.
// ─────────────────────────────────────────────────────────────────────────────
bool GameManager::getQuestion(const std::string& username, Question& outQuestion)
{
    auto stateIt = m_playerStates.find(username);
    if (stateIt == m_playerStates.end())
        return false;

    PlayerGameState& state = stateIt->second;

    auto roomIt = m_playerRoom.find(username);
    if (roomIt == m_playerRoom.end())
        return false;

    int roomId = roomIt->second;

    auto questionsIt = m_roomQuestions.find(roomId);
    if (questionsIt == m_roomQuestions.end())
        return false;

    const std::vector<Question>& questions = questionsIt->second;

    if (state.currentQuestionIndex >= (int)questions.size())
    {
        // Player has answered all questions
        state.finished = true;
        return false;
    }

    outQuestion = questions[state.currentQuestionIndex];
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// submitAnswer
// Records whether the player answered correctly and how long it took.
// Advances their question index.
// ─────────────────────────────────────────────────────────────────────────────
void GameManager::submitAnswer(const std::string& username, int answerId, float answerTime)
{
    auto stateIt = m_playerStates.find(username);
    if (stateIt == m_playerStates.end())
        return;

    PlayerGameState& state = stateIt->second;

    auto roomIt = m_playerRoom.find(username);
    if (roomIt == m_playerRoom.end())
        return;

    int roomId = roomIt->second;

    auto questionsIt = m_roomQuestions.find(roomId);
    if (questionsIt == m_roomQuestions.end())
        return;

    const std::vector<Question>& questions = questionsIt->second;

    if (state.currentQuestionIndex >= (int)questions.size())
    {
        state.finished = true;
        return;
    }

    const Question& currentQ = questions[state.currentQuestionIndex];

    // correctAnswerId is always 0 (see SqliteDataBase::getQuestions)
    if (answerId == currentQ.getCorrectAnswerId())
        state.correctAnswers++;
    else
        state.wrongAnswers++;

    state.totalAnswerTime += answerTime;
    state.answersSubmitted++;

    // Move to next question
    state.currentQuestionIndex++;

    if (state.currentQuestionIndex >= (int)questions.size())
        state.finished = true;

    std::cout << "[GameManager] " << username
        << " answered question " << state.currentQuestionIndex
        << " (answerId=" << answerId << ", time=" << answerTime << "s)"
        << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// isGameFinished
// True when every player who started the game has finished all questions.
// ─────────────────────────────────────────────────────────────────────────────
bool GameManager::isGameFinished(int roomId) const
{
    auto it = m_roomPlayers.find(roomId);
    if (it == m_roomPlayers.end())
        return true; // no game = treat as finished

    for (const std::string& username : it->second)
    {
        auto stateIt = m_playerStates.find(username);
        if (stateIt == m_playerStates.end())
            continue; // player was removed (left mid-game), skip

        if (!stateIt->second.finished)
            return false;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// getGameResults
// Collects results for all players in the room.
// Persists statistics to the DB on first call when the game is finished.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<PlayerResults> GameManager::getGameResults(int roomId)
{
    auto it = m_roomPlayers.find(roomId);
    if (it == m_roomPlayers.end())
        return {};

    std::vector<PlayerResults> results;

    for (const std::string& username : it->second)
    {
        auto stateIt = m_playerStates.find(username);
        if (stateIt == m_playerStates.end())
            continue;

        const PlayerGameState& state = stateIt->second;

        PlayerResults r;
        r.username = username;
        r.correctAnswersCount = state.correctAnswers;
        r.wrongAnswerCount = state.wrongAnswers;

        if (state.answersSubmitted > 0)
            r.averageAnswerTime = state.totalAnswerTime / (float)state.answersSubmitted;
        else
            r.averageAnswerTime = 0.0f;

        results.push_back(r);
    }

    // Persist to DB if the game is fully finished
    if (isGameFinished(roomId))
        persistResults(roomId);

    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// removePlayer
// Removes a single player from the active game (LeaveGame path).
// Their results are still visible in getGameResults.
// ─────────────────────────────────────────────────────────────────────────────
void GameManager::removePlayer(const std::string& username)
{
    auto roomIt = m_playerRoom.find(username);
    if (roomIt == m_playerRoom.end())
        return;

    int roomId = roomIt->second;

    // Mark as finished so isGameFinished() still works correctly
    auto stateIt = m_playerStates.find(username);
    if (stateIt != m_playerStates.end())
        stateIt->second.finished = true;

    // Remove from playerRoom map
    m_playerRoom.erase(roomIt);

    std::cout << "[GameManager] " << username
        << " removed from active game in room " << roomId << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// gameExists
// ─────────────────────────────────────────────────────────────────────────────
bool GameManager::gameExists(int roomId) const
{
    return m_roomQuestions.find(roomId) != m_roomQuestions.end();
}

// ─────────────────────────────────────────────────────────────────────────────
// persistResults  (private)
// Writes each player's results to STATISTICS and GAME_RESULTS tables.
// ─────────────────────────────────────────────────────────────────────────────
void GameManager::persistResults(int roomId)
{
    auto it = m_roomPlayers.find(roomId);
    if (it == m_roomPlayers.end())
        return;

    for (const std::string& username : it->second)
    {
        auto stateIt = m_playerStates.find(username);
        if (stateIt == m_playerStates.end())
            continue;

        const PlayerGameState& state = stateIt->second;

        float avgTime = (state.answersSubmitted > 0)
            ? state.totalAnswerTime / (float)state.answersSubmitted
            : 0.0f;

        try
        {
            m_database->submitGameStatistics(username,
                state.correctAnswers,
                state.wrongAnswers,
                avgTime,
                roomId);
        }
        catch (const std::exception& e)
        {
            std::cerr << "[GameManager] Failed to persist stats for "
                << username << ": " << e.what() << std::endl;
        }
    }

    std::cout << "[GameManager] Results persisted for room " << roomId << std::endl;
}