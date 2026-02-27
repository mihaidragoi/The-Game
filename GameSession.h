#pragma once

#include <string>
#include <vector>
#include <queue>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <crow/websocket.h>
#include <memory>          
#include "GameLogic.h"     

struct GameSession {
    int id;
    std::string pin;
    std::vector<std::string> players;
    bool started = false;
    std::chrono::steady_clock::time_point creationTime;
    mutable std::mutex mtx;
    bool isPublic = true;
    std::unique_ptr<GameLogic> gameLogic;
    std::unordered_map<std::string, crow::websocket::connection*> wsConnections;
    bool isFireMode = false;

    std::vector<std::string> chatMessages;

    bool isAvailableForRandom() const;
    GameSession(int sessionId, std::string_view pin = "", bool publicGame = true);

    bool canJoin() const;
    bool hasMinimumPlayers() const;
    bool shouldStartNow() const;
    void finalizeJoining();
    void initializeGame();

    void setGameMode(bool fireMode);

    bool matchesPin(std::string_view pin) const;

    void addMessage(const std::string& msg);
    std::vector<std::string> getMessages() const;
};