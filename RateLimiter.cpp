#include "RateLimiter.h"

bool RateLimiter::allow(const std::string& ip) {
    using namespace std::chrono;

    const auto now = steady_clock::now();
    const auto LIMIT = 300ms;   

    auto it = lastRequest.find(ip);
    if (it != lastRequest.end()) {
        auto diff = now - it->second;

        if (diff < LIMIT) {
            return false; 
        }
    }

    lastRequest[ip] = now;
    return true;
}
