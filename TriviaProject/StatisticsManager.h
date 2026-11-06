#pragma once
#include <vector>
#include <string>
#include "IDatabase.h"

class StatisticsManager
{
public:
    static StatisticsManager& getInstance(IDatabase* database)
    {
        static StatisticsManager instance(database);
        return instance;
    }

    StatisticsManager(const StatisticsManager&) = delete;
    StatisticsManager& operator=(const StatisticsManager&) = delete;

    std::vector<std::string> getHighScore();
    std::vector<std::string> getUserStatistics(const std::string& username);

private:
    explicit StatisticsManager(IDatabase* database);

    IDatabase* m_database;
};