#include "GameLobby.h"
#include "Config.h"
#include <iostream>

GameLobby & GameLobby::instance() {
    static GameLobby instance;
    return instance;
}

std::vector<int> GameLobby::getGamesToStartAutomatically() {
    std::lock_guard lock(lobbyMutex);
    std::vector<int> ready;
    for (const auto& [id, session] : sessions) {
        std::lock_guard sLock(session->mtx);
        if (!session->started && session->shouldStartNow()) {
            ready.push_back(id);
        }
    }
    return ready;
}

std::optional<int> GameLobby::findRandomGame() {
    std::lock_guard lock(lobbyMutex);
    for (auto& [id, session] : sessions) {
        std::lock_guard sLock(session->mtx);
        if (session->isAvailableForRandom()) {
            return id;
        }
    }
    return std::nullopt;
}

std::optional<int> GameLobby::createGame(const std::string& username, std::string_view pin, bool isPublic) {
    std::lock_guard lock(lobbyMutex);

    int newId = nextSessionId++;
    auto newSession = std::make_shared<GameSession>(newId, pin, isPublic);
    newSession->players.push_back(username); 
    sessions[newId] = newSession;

    std::cout << "[LOBBY] Created game " << newId << " with PIN '" << pin << "' for user '" << username << "'" << std::endl;
    return newId;
}

std::optional<int> GameLobby::findGame(std::string_view pin) {
    std::lock_guard lock(lobbyMutex);
    for (auto& [id, session] : sessions) {
        std::lock_guard sLock(session->mtx);
        if (!session->started && session->matchesPin(pin)) {
            return id;
        }
    }
    return std::nullopt;
}

void GameLobby::registerWebSocket(int gameId, const std::string& username, crow::websocket::connection* conn) {
    auto session = getSession(gameId);
    if (session) {
        std::lock_guard lock(session->mtx);
        session->wsConnections[username] = conn;
    }
}

bool GameLobby::joinGame(int gameId, const std::string& username) {
    auto session = getSession(gameId);
    if (!session) return false;

    std::lock_guard lock(session->mtx);
    if (session->started || session->players.size() >= Config::MAX_PLAYERS) return false;

    session->players.push_back(username);
    std::cout << "[LOBBY] User '" << username << "' joined game " << gameId << std::endl;
    return true;
}

void GameLobby::startGame(int gameId) {
    auto session = getSession(gameId);
    if (!session) return;

    std::lock_guard lock(session->mtx);
    if (session->started) return;

    session->initializeGame();

    session->started = true;
    std::cout << "[LOBBY] Game " << gameId << " started with " << session->players.size() << " players" << std::endl;
}

bool GameLobby::leaveGame(int gameId, const std::string& username) {
    std::lock_guard lock(lobbyMutex);

    auto it = sessions.find(gameId);
    if (it == sessions.end())
        return false;

    auto session = it->second;
    std::lock_guard sLock(session->mtx);

    auto& players = session->players;
    auto pIt = std::find(players.begin(), players.end(), username);
    if (pIt == players.end())
        return false;

    players.erase(pIt);

    if (players.empty()) {
        sessions.erase(it);
    }

    return true;
}

std::shared_ptr<GameSession> GameLobby::getSession(int gameId) {
    std::lock_guard lock(lobbyMutex);
    auto it = sessions.find(gameId);
    return (it != sessions.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<GameSession>> GameLobby::getAllSessions() {
    std::lock_guard lock(lobbyMutex);
    std::vector<std::shared_ptr<GameSession>> result;
    for (auto& [id, session] : sessions)
        result.push_back(session);
    return result;
}
