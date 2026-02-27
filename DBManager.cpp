#include "DBManager.h"
#include <iostream>
#include <filesystem>
#include <sqlite_orm/sqlite_orm.h>
using namespace sqlite_orm;

auto& DBManager::storage() {
    static bool first = true;
    if (first) {
        std::cout << "Database file will be created at: "
            << std::filesystem::current_path().string() << "\\thegame.db"
            << std::endl;
        first = false;
    }
    static auto storage = make_storage(
        "thegame.db",
        make_table("users",
            make_column("id", &User::id, primary_key().autoincrement()),
            make_column("username", &User::username, unique()),
            make_column("password_hash", &User::passwordHash),
            make_column("salt", &User::salt),
            make_column("games_played", &User::gamesPlayed),
            make_column("games_won", &User::gamesWon),
            make_column("hours_played", &User::hoursPlayed),
            make_column("performance_score", &User::performanceScore)
        )
    );
    return storage;
}

bool DBManager::userExists(const std::string& username) {
    auto& db = storage();
    auto results = db.get_all<User>(where(c(&User::username) == username));
    return !results.empty();
}

bool DBManager::addUser(const std::string& username, const std::string& passwordHash, const std::string& salt) {
    if (userExists(username))
        return false;

    User user;
    user.username = username;
    user.passwordHash = passwordHash;
    user.salt = salt;
    storage().insert(user);
    return true;
}

std::optional<User> DBManager::getUser(const std::string& username) {
    auto& db = storage();
    auto results = db.get_all<User>(where(c(&User::username) == username));

    if (results.empty())
        return std::nullopt;

    return results.front();
}  

std::vector<User> DBManager::getAllUsers() {
    auto& db = storage();
    return db.get_all<User>();
}

void DBManager::updateUser(const User& user) {
    storage().update(user);
}

void DBManager::init() {
    storage().sync_schema();
}



