#include "NetworkClient.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>


size_t NetworkClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


bool NetworkClient::RegisterUser(const std::string& username, const std::string& password) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    nlohmann::json data = { {"username", username}, {"password", password} };
    std::string body = data.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18080/register");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}

bool NetworkClient::LoginUser(const std::string& username, const std::string& password) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    nlohmann::json data = { {"username", username}, {"password", password} };
    std::string body = data.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18080/login");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}


int NetworkClient::CreateGame(const std::string& username, const std::string& pin) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    nlohmann::json data = { {"username", username}, {"pin", pin} };
    std::string body = data.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18080/create_game");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return -1;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.value("success", false)) {
            return j["gameId"].get<int>();
        }
    }
    catch (...) {}
    return -1;
}

int NetworkClient::JoinGame(const std::string& username, const std::string& pin) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    nlohmann::json data = { {"username", username}, {"pin", pin} };
    std::string body = data.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18080/joinByPin");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return -1;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.value("success", false)) {
            return j["gameId"].get<int>();
        }
    }
    catch (...) {}
    return -1;
}

int NetworkClient::JoinRandomGame(const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;

    nlohmann::json data = { {"username", username} };
    std::string body = data.dump();
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:18080/joinLobby");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return -1;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.value("success", false)) {
            return j["gameId"].get<int>();
        }
    }
    catch (...) {}
    return -1;
}


std::vector<std::string> NetworkClient::GetMessages(const std::string& gameId) {
    CURL* curl = curl_easy_init();
    if (!curl) return {};
    std::string url = "http://localhost:18080/games/" + gameId + "/messages";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return {};
    try {
        nlohmann::json j = nlohmann::json::parse(response);
        std::vector<std::string> messages;
        if (j.contains("messages")) {
            for (const auto& msg : j["messages"]) {
                messages.push_back(msg.get<std::string>());
            }
        }
        return messages;
    }
    catch (...) { return {}; }
}

bool NetworkClient::SendMessage(const std::string& gameId, const std::string& message, const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    nlohmann::json data = { {"message", message}, {"username", username} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/games/" + gameId + "/sendMessage";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;
    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}

bool NetworkClient::PlayCard(const std::string& gameId, int cardValue, int stackIndex, const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    nlohmann::json data = { {"cardValue", cardValue}, {"stackIndex", stackIndex}, {"username", username} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/games/" + gameId + "/playCard";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;
    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}

bool NetworkClient::EndTurn(const std::string& gameId, const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    nlohmann::json data = { {"username", username} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/games/" + gameId + "/endTurn";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;
    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}

bool NetworkClient::StartGame(const std::string& gameId) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    int gId = std::stoi(gameId);
    nlohmann::json data = { {"gameId", gId} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/startGame";
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;
    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}

GameState NetworkClient::GetGameState(const std::string& gameId, const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return {};

    std::string url = "http://localhost:18080/games/" + gameId + "/state?username=" + username;
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200) return {};

    if (response.empty() || response[0] != '{') return {};

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        GameState gameState;
        if (j.contains("stacks")) gameState.m_stacks = j["stacks"].get<std::vector<int>>();
        if (j.contains("playerHand")) gameState.m_playerHand = j["playerHand"].get<std::vector<int>>();
        if (j.contains("isMyTurn")) gameState.m_isMyTurn = j["isMyTurn"].get<bool>();
        if (j.contains("status")) gameState.m_status = j["status"].get<std::string>();
        if (j.contains("deckCount")) gameState.m_deckCount = j["deckCount"].get<int>();

        if (j.contains("fireTargets")) gameState.m_fireTargets = j["fireTargets"].get<std::vector<int>>();
        if (j.contains("fireMode")) gameState.m_fireMode = j["fireMode"].get<bool>();

        return gameState;
    }
    catch (...) { return {}; }
}

bool NetworkClient::LeaveGame(const std::string& gameId, const std::string& username) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    nlohmann::json data = { {"username", username} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/games/" + gameId + "/leaveGame";

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) {
        return false;
    }
}

bool NetworkClient::SetGameMode(const std::string& gameId, bool fireMode) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    nlohmann::json data = { {"fireMode", fireMode} };
    std::string body = data.dump();
    std::string url = "http://localhost:18080/games/" + gameId + "/setMode";
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    try {
        nlohmann::json j = nlohmann::json::parse(response);
        return j.value("success", false);
    }
    catch (...) { return false; }
}