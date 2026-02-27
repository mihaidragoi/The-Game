#pragma once
#include <string>

class PasswordService {
public:
    static std::string generateSalt();
    static std::string hashPassword(const std::string& password, const std::string& salt);
    static std::pair<std::string, std::string> hashPasswordWithSalt(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt);
};