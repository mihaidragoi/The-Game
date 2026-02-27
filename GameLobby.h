#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include "GameSession.h"

class GameLobby {
public:
    static GameLobby& instance();
    std::vector<int> getGamesToStartAutomatically();
    std::optional<int> findRandomGame();
    std::optional<int> createGame(const std::string& username, std::string_view pin = "", bool isPublic = true);
    std::optional<int> findGame(std::string_view pin);
    bool joinGame(int gameId, const std::string& username);
    void startGame(int gameId);
    bool leaveGame(int gameId, const std::string& username);
    void registerWebSocket(int gameId, const std::string& username, crow::websocket::connection* conn);

    
    std::shared_ptr<GameSession> getSession(int gameId);
    std::vector<std::shared_ptr<GameSession>> getAllSessions();

private:
    GameLobby() = default;
    std::unordered_map<int, std::shared_ptr<GameSession>> sessions;
    std::mutex lobbyMutex;
    int nextSessionId = 1;
};
