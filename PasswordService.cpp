#include "PasswordService.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "../SecurityLib/SecurityEngine.h"


std::string PasswordService::generateSalt() {
        return SecurityEngine::GenerateSalt();
}

std::string PasswordService::hashPassword(const std::string& password, const std::string& salt) {
	return SecurityEngine::ComputeHash(password, salt);
}

std::pair<std::string, std::string> PasswordService::hashPasswordWithSalt(const std::string& password) {
    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);
    return { hash, salt };
}

bool PasswordService::verifyPassword(const std::string& password, const std::string& hashedPassword, const std::string& salt) {
    std::string newHash = hashPassword(password, salt);
    std::cout << "DEBUG: Input password = '" << password << "'" << std::endl;
    std::cout << "DEBUG: Salt = '" << salt << "'" << std::endl;
    std::cout << "DEBUG: Generated hash = '" << newHash << "'" << std::endl;
    std::cout << "DEBUG: Stored hash   = '" << hashedPassword << "'" << std::endl;
    std::cout << "DEBUG: Match? " << (newHash == hashedPassword ? "YES" : "NO") << std::endl;
    return newHash == hashedPassword;
}