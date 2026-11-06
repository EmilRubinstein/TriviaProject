#include "StatisticsManager.h"

StatisticsManager::StatisticsManager(IDatabase* database)
    : m_database(database)
{
}

std::vector<std::string> StatisticsManager::getHighScore()
{
    std::vector<std::string> m;
    try
    {
        m = m_database->getHighScores();
    }
    catch (const std::exception& e)
    {
        m.clear();
        m.push_back("No Scores found for the user ");
    }
    return m;
}

std::vector<std::string> StatisticsManager::getUserStatistics(const std::string& username)
{
    std::vector<std::string> stats;

    try
    {
        stats.push_back("Username: " + username);
        stats.push_back("Games played: " + std::to_string(m_database->getNumOfPlayerGames(username)));
        stats.push_back("Correct answers: " + std::to_string(m_database->getNumOfCorrectAnswers(username)));
        stats.push_back("Total answers: " + std::to_string(m_database->getNumOfTotalAnswers(username)));
        stats.push_back("Avg answer time: " + std::to_string(m_database->getPlayerAverageAnswerTime(username)));
        stats.push_back("Score: " + std::to_string(m_database->getPlayerScore(username)));
    }
    catch (const std::exception& e)
    {
        stats.clear();
        stats.push_back("No statistics found for: " + username);
    }

    return stats;
}