#pragma once
#include <string>

struct User {
    int id = 0;
    std::string username;
    std::string passwordHash;
    std::string salt;
    int gamesPlayed = 0;
    int gamesWon = 0;                                                       
    double hoursPlayed = 0.0;
    float performanceScore = 0.0f;
};