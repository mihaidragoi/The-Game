#pragma once
#include <string>
#include <optional>
#include <vector>
#include <sqlite_orm/sqlite_orm.h>
#include "User.h"

class DBManager {
public:
  
    static void init();

    static bool addUser(const std::string& username, const std::string& passwordHash, const std::string& salt);
    static std::optional<User> getUser(const std::string& username);
    static bool userExists(const std::string& username);
    static std::vector<User> getAllUsers();

    static void updateUser(const User& user);

private:

    static auto& storage();
};