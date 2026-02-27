#include "GameSession.h"
#include <algorithm>

GameSession::GameSession(int sessionId, std::string_view pin, bool publicGame)
    : id(sessionId), pin(pin), isPublic(publicGame), creationTime(std::chrono::steady_clock::now()) {
}

bool GameSession::canJoin() const {
    return !started && players.size() < 5;
}

bool GameSession::hasMinimumPlayers() const {
    return players.size() >= 2;
}

bool GameSession::shouldStartNow() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - creationTime);

    bool isFull = players.size() >= 5;

    bool timeExpiredAndMinPlayers = (elapsed.count() >= 30 && hasMinimumPlayers());

    return isFull || timeExpiredAndMinPlayers;
}

bool GameSession::isAvailableForRandom() const {
    return isPublic && !started && players.size() < 5;
}

void GameSession::finalizeJoining() {

}

void GameSession::addMessage(const std::string& msg) {
    chatMessages.push_back(msg);
    if (chatMessages.size() > 50) {
        chatMessages.erase(chatMessages.begin());
    }
}

std::vector<std::string> GameSession::getMessages() const {
    return chatMessages;
}

bool GameSession::matchesPin(std::string_view pin) const {
    if (pin.empty()) return true;
    return this->pin == pin;
}

void GameSession::setGameMode(bool fireMode) {
    isFireMode = fireMode;
    if (gameLogic) {
        gameLogic->SetFireMode(fireMode);
    }
}

void GameSession::initializeGame() {
    gameLogic = std::make_unique<GameLogic>(players);
    gameLogic->SetFireMode(isFireMode);
    gameLogic->Initialize();
}