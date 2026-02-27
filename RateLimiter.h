#pragma once
#include <unordered_map>
#include <chrono>
#include <string>

class RateLimiter {
public:
    static bool allow(const std::string& ip);

private:
    static inline std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastRequest;
};

