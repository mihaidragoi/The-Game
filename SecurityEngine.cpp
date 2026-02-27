#include "pch.h"
#include "SecurityEngine.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string SecurityEngine::BytesToHex(const unsigned char* data, size_t len)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::string SecurityEngine::GenerateSalt()
{
    unsigned char salt[16];
    if (!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Failed to generate salt");
    }
    return BytesToHex(salt, sizeof(salt));
}

std::string SecurityEngine::ComputeHash(const std::string& input, const std::string& salt)
{
    std::string salted = input + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted.data()), salted.size(), hash);
    return BytesToHex(hash, SHA256_DIGEST_LENGTH);
}

std::string SecurityEngine::EncryptData(const std::string& data, const std::string& key)
{
	std::string output = data;
    for (size_t i = 0; i < data.size(); ++i) 
        output[i] = data[i] ^ key[i % key.size()];
	return output;
}

