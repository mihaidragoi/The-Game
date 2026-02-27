#pragma once
#include "SecurityExports.h"
#include <string>

class SECURITYLIB_API SecurityEngine
{
public:
	static std::string BytesToHex(const unsigned char* data, size_t len);
	static std::string GenerateSalt();
	static std::string ComputeHash(const std::string& input, const std::string& salt);
	static std::string EncryptData(const std::string& data, const std::string& key);
};
