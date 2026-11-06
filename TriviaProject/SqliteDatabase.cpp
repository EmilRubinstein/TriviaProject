#include "SqliteDataBase.h"
#include <iostream>
#include <stdexcept>

//____________________________________________________ CONSTRUCTORS ___________________________________________________________

SqliteDataBase::SqliteDataBase() : _db(nullptr) {
    if (sqlite3_open("trivia.db", &_db) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(_db);
        sqlite3_close(_db);
        throw std::runtime_error("[DB] Failed to open: " + err);
    }
    std::cout << "[DB] Opened trivia.db" << std::endl;
    createTables();
    initQuestionsTable();
}

SqliteDataBase::~SqliteDataBase() {
    if (_db) {
        sqlite3_close(_db);
        _db = nullptr;
    }
}

//____________________________________________________ GETTERS ___________________________________________________________

std::vector<Question> SqliteDataBase::getQuestions(int amount)
{
    std::vector<Question> questions;

    // LIMIT by amount, ORDER BY RANDOM so each game gets different questions
    std::string sql =
        "SELECT question, correct_answer, wrong_answer1, wrong_answer2, wrong_answer3 "
        "FROM QUESTIONS ORDER BY RANDOM() LIMIT " + std::to_string(amount) + ";";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("[DB] Failed to prepare getQuestions query");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // Extract each column as string
        std::string question = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string correctAnswer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string wrong1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string wrong2 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string wrong3 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // Build possible answers — correct answer always at index 0
        std::vector<std::string> possibleAnswers = {
            correctAnswer,
            wrong1,
            wrong2,
            wrong3
        };

        // correctAnswerId = 0 since correct is always first
        questions.push_back(Question(question, possibleAnswers, 0));
    }

    sqlite3_finalize(stmt);
    return questions;
}

float SqliteDataBase::getPlayerAverageAnswerTime(const std::string& username)
{
    const char* sql =
        "SELECT S.avg_answer_time FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "WHERE U.username = ?;";

    sqlite3_stmt* stmt = nullptr;
    float result = 0.0f;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getPlayerAverageAnswerTime");

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = static_cast<float>(sqlite3_column_double(stmt, 0));

    sqlite3_finalize(stmt);
    return result;
}

int SqliteDataBase::getNumOfCorrectAnswers(const std::string& username)
{
    const char* sql =
        "SELECT S.correct_answers FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "WHERE U.username = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = 0;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getNumOfCorrectAnswers");

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return result;
}

int SqliteDataBase::getNumOfTotalAnswers(const std::string& username)
{
    const char* sql =
        "SELECT S.total_answers FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "WHERE U.username = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = 0;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getNumOfTotalAnswers");

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return result;
}

int SqliteDataBase::getNumOfPlayerGames(const std::string& username)
{
    const char* sql =
        "SELECT S.games_played FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "WHERE U.username = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = 0;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getNumOfPlayerGames");

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return result;
}

int SqliteDataBase::getPlayerScore(const std::string& username)
{
    const char* sql =
        "SELECT S.score FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "WHERE U.username = ?;";

    sqlite3_stmt* stmt = nullptr;
    int result = 0;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getPlayerScore");

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = static_cast<int>(sqlite3_column_double(stmt, 0));

    sqlite3_finalize(stmt);
    return result;
}

std::vector<std::string> SqliteDataBase::getHighScores()
{
    // Returns top 5 players formatted as "username: score"
    const char* sql =
        "SELECT U.username, S.score FROM STATISTICS S "
        "JOIN USERS U ON S.user_id = U.id "
        "ORDER BY S.score DESC LIMIT 5;";

    sqlite3_stmt* stmt = nullptr;
    std::vector<std::string> result;

    if (sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("[DB] Failed to prepare getHighScores");

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int score = static_cast<int>(sqlite3_column_double(stmt, 1));
        result.push_back(username + ": " + std::to_string(score));
    }

    sqlite3_finalize(stmt);
    return result;
}

//____________________________________________________ INITS ___________________________________________________________

void SqliteDataBase::initQuestionsTable()
{
    const char* countSql = "SELECT COUNT(*) FROM QUESTIONS;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(_db, countSql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw std::runtime_error("[DB] Failed to prepare count query");
    }

    int count = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    // Do not insert again if questions already exist
    if (count > 0)
    {
        std::cout << "[DB] Questions already initialized." << std::endl;
        return;
    }

    const char* insertSql =
        "INSERT INTO QUESTIONS "
        "(question, correct_answer, wrong_answer1, wrong_answer2, wrong_answer3) VALUES "

        "('What color is the \"black box\" in an airplane?',"
        "'Orange', 'Black', 'White', 'Silver'),"

        "('How long did the 100 Years War last?',"
        "'116 years', '100 years', '99 years', '150 years'),"

        "('Which animal can not jump?',"
        "'Elephant', 'Dog', 'Cat', 'Tiger'),"

        "('What is technically a tomato?',"
        "'Fruit', 'Vegetable', 'Meat', 'Berry'),"

        "('Which country invented pizza?',"
        "'Italy', 'France', 'Germany', 'USA'),"

        "('What do pandas mostly eat?',"
        "'Bamboo', 'Fish', 'Meat', 'Grass'),"

        "('How many moons does Earth have?',"
        "'1', '2', '5', '0'),"

        "('What is the main ingredient in guacamole?',"
        "'Avocado', 'Apple', 'Potato', 'Banana'),"

        "('Which bird is famous for repeating words?',"
        "'Parrot', 'Crow', 'Penguin', 'Eagle'),"

        "('What happens if Pinocchio says \"My nose will grow now\"?',"
        "'Paradox', 'Nothing', 'It explodes', 'He sleeps');";

    char* errMsg = nullptr;

    if (sqlite3_exec(_db, insertSql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string err = errMsg;
        sqlite3_free(errMsg);

        throw std::runtime_error("[DB] Failed to insert questions: " + err);
    }

    std::cout << "[DB] Questions initialized successfully." << std::endl;
}

void SqliteDataBase::createTables() {
    const char* sql =
        // USERS table
        "CREATE TABLE IF NOT EXISTS USERS ("
        "   id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   username TEXT NOT NULL UNIQUE,"
        "   password TEXT NOT NULL,"
        "   email    TEXT NOT NULL"
        ");"

        // QUESTIONS table
        "CREATE TABLE IF NOT EXISTS QUESTIONS ("
        "   id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   question       TEXT NOT NULL,"
        "   correct_answer TEXT NOT NULL,"
        "   wrong_answer1  TEXT NOT NULL,"
        "   wrong_answer2  TEXT NOT NULL,"
        "   wrong_answer3  TEXT NOT NULL"
        ");"

        // STATISTICS table
        "CREATE TABLE IF NOT EXISTS STATISTICS ("
        "   id               INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   user_id          INTEGER NOT NULL,"
        "   correct_answers  INTEGER DEFAULT 0,"
        "   wrong_answers    INTEGER DEFAULT 0,"
        "   total_answers    INTEGER DEFAULT 0,"
        "   games_played     INTEGER DEFAULT 0,"
        "   avg_answer_time  REAL DEFAULT 0.0,"
        "   score            REAL DEFAULT 0.0,"
        "   FOREIGN KEY (user_id) REFERENCES USERS(id) ON DELETE CASCADE"
        ");"

        // ROOMS table
        "CREATE TABLE IF NOT EXISTS ROOMS ("
        "   id                INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   room_name         TEXT NOT NULL,"
        "   creator_id        INTEGER NOT NULL,"
        "   max_players       INTEGER DEFAULT 4,"
        "   time_per_question INTEGER DEFAULT 30,"
        "   is_active         INTEGER DEFAULT 1,"
        "   FOREIGN KEY (creator_id) REFERENCES USERS(id) ON DELETE CASCADE"
        ");"

        // ROOM_PLAYERS table
        "CREATE TABLE IF NOT EXISTS ROOM_PLAYERS ("
        "   room_id INTEGER NOT NULL,"
        "   user_id INTEGER NOT NULL,"
        "   PRIMARY KEY (room_id, user_id),"
        "   FOREIGN KEY (room_id) REFERENCES ROOMS(id) ON DELETE CASCADE,"
        "   FOREIGN KEY (user_id) REFERENCES USERS(id) ON DELETE CASCADE"
        ");"

        // GAME_RESULTS table
        "CREATE TABLE IF NOT EXISTS GAME_RESULTS ("
        "   id               INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   room_id          INTEGER NOT NULL,"
        "   user_id          INTEGER NOT NULL,"
        "   correct_answers  INTEGER DEFAULT 0,"
        "   wrong_answers    INTEGER DEFAULT 0,"
        "   avg_answer_time  REAL DEFAULT 0.0,"
        "   played_at        TEXT DEFAULT CURRENT_TIMESTAMP,"
        "   FOREIGN KEY (room_id) REFERENCES ROOMS(id) ON DELETE CASCADE,"
        "   FOREIGN KEY (user_id) REFERENCES USERS(id) ON DELETE CASCADE"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(_db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("[DB] Failed to create tables: " + err);
    }

    sqlite3_exec(_db, "DELETE FROM ROOMS;", nullptr, nullptr, nullptr);
    sqlite3_exec(_db, "DELETE FROM ROOM_PLAYERS;", nullptr, nullptr, nullptr);

    std::cout << "[DB] All tables created successfully." << std::endl;
}

//____________________________________________________ CHECKS ___________________________________________________________

bool SqliteDataBase::doesUserExist(const std::string& username) {
    const char* sql = "SELECT COUNT(*) FROM USERS WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count > 0;
}

bool SqliteDataBase::doesPasswordMatch(const std::string& username, const std::string& password) {
    const char* sql = "SELECT COUNT(*) FROM USERS WHERE username = ? AND password = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count > 0;
}

//____________________________________________________ ADD ___________________________________________________________

int SqliteDataBase::createRoom(const RoomData& roomData, const LoggedUser& user)
{
    const char* sql =
        "INSERT INTO ROOMS "
        "(room_name, creator_id, max_players, time_per_question, is_active) "
        "VALUES "
        "(?, (SELECT id FROM USERS WHERE username = ?), ?, ?, 1);";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, roomData.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.getUsername().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, roomData.maxPlayers);
    sqlite3_bind_int(stmt, 4, roomData.timePerQuestion);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(_db);

        sqlite3_finalize(stmt);

        throw std::runtime_error(err);
    }

    sqlite3_finalize(stmt);

    int roomId = (int)sqlite3_last_insert_rowid(_db);

    const char* addPlayerSql =
        "INSERT INTO ROOM_PLAYERS (room_id, user_id) "
        "VALUES (?, (SELECT id FROM USERS WHERE username = ?));";

    sqlite3_stmt* playerStmt = nullptr;
    sqlite3_prepare_v2(_db, addPlayerSql, -1, &playerStmt, nullptr);
    sqlite3_bind_int(playerStmt, 1, roomId);
    sqlite3_bind_text(playerStmt, 2, user.getUsername().c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(playerStmt) != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(_db);
        sqlite3_finalize(playerStmt);
        throw std::runtime_error("[DB] Failed to add creator to ROOM_PLAYERS: " + err);
    }

    sqlite3_finalize(playerStmt);

    return roomId;
}

void SqliteDataBase::addNewUser(const std::string& username, const std::string& password, const std::string& email)
{
    const char* sql =
        "INSERT INTO USERS (username, password, email) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(_db);

        sqlite3_finalize(stmt);

        throw std::runtime_error("[DB] addNewUser failed: " + err);
    }

    sqlite3_finalize(stmt);

    // Get new user's ID
    int userId = (int)sqlite3_last_insert_rowid(_db);

    // Create statistics row
    const char* statSql =
        "INSERT INTO STATISTICS (user_id) VALUES (?);";

    sqlite3_stmt* statStmt = nullptr;

    sqlite3_prepare_v2(_db, statSql, -1, &statStmt, nullptr);

    sqlite3_bind_int(statStmt, 1, userId);

    if (sqlite3_step(statStmt) != SQLITE_DONE)
    {
        std::string err = sqlite3_errmsg(_db);

        sqlite3_finalize(statStmt);

        throw std::runtime_error("[DB] Failed to create statistics row: " + err);
    }

    sqlite3_finalize(statStmt);
}

void SqliteDataBase::deleteRoom(int roomId)
{
    const char* sql = "DELETE FROM ROOMS WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, roomId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void SqliteDataBase::submitGameStatistics(const std::string& username,
    int correctAnswers,
    int wrongAnswers,
    float avgAnswerTime,
    int roomId)
{
    // Update STATISTICS table
    const char* sql =
        "UPDATE STATISTICS SET "
        "correct_answers = correct_answers + ?, "
        "wrong_answers = wrong_answers + ?, "
        "total_answers = total_answers + ?, "
        "games_played = games_played + 1, "
        "avg_answer_time = (avg_answer_time * games_played + ?) / (games_played + 1), "
        "score = score + ? "
        "WHERE user_id = (SELECT id FROM USERS WHERE username = ?);";

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, correctAnswers);
    sqlite3_bind_int(stmt, 2, wrongAnswers);
    sqlite3_bind_int(stmt, 3, correctAnswers + wrongAnswers);
    sqlite3_bind_double(stmt, 4, avgAnswerTime);
    sqlite3_bind_int(stmt, 5, correctAnswers * 10); // Score calculation
    sqlite3_bind_text(stmt, 6, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Insert into GAME_RESULTS table
    const char* insertSql =
        "INSERT INTO GAME_RESULTS (room_id, user_id, correct_answers, wrong_answers, avg_answer_time) "
        "VALUES (?, (SELECT id FROM USERS WHERE username = ?), ?, ?, ?);";

    sqlite3_stmt* insertStmt = nullptr;
    sqlite3_prepare_v2(_db, insertSql, -1, &insertStmt, nullptr);
    sqlite3_bind_int(insertStmt, 1, roomId);
    sqlite3_bind_text(insertStmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(insertStmt, 3, correctAnswers);
    sqlite3_bind_int(insertStmt, 4, wrongAnswers);
    sqlite3_bind_double(insertStmt, 5, avgAnswerTime);
    sqlite3_step(insertStmt);
    sqlite3_finalize(insertStmt);
}