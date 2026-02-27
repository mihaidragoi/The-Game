#include "RouteHandler.h"
#include "DBManager.h"
#include "GameLobby.h"
#include "Config.h"
#include <regex>
#include "PasswordService.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

static int getPlayerIndex(const std::vector<std::string>& players, const std::string& username) {
    auto it = std::find(players.begin(), players.end(), username);
    if (it != players.end()) {
        return std::distance(players.begin(), it);
    }
    return -1;
}

static void updateStatsAfterGame(GameSession* session, const std::string& status) {
    if (status == "Running") return;

    bool won = (status == "Won");

    auto now = std::chrono::steady_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - session->creationTime).count();
    double hoursAdded = static_cast<double>(durationMs) / 3600000.0;

    int totalCardsInHand = 0;
    if (!won && session->gameLogic) {
        for (int i = 0; i < session->players.size(); ++i) {
            totalCardsInHand += session->gameLogic->GetPlayerHand(i).size();
        }
    }

    double matchScore;

    if (won) {
        matchScore = 5.0;
    }
    else {
        matchScore = 5.0 - (totalCardsInHand * 0.1);
        if (matchScore < 1.0) matchScore = 1.0;
    }

    for (const auto& username : session->players) {
        auto userOpt = DBManager::getUser(username);
        if (userOpt.has_value()) {
            User user = userOpt.value();

            int oldGamesCount = user.gamesPlayed;
            user.gamesPlayed++;

            if (won) user.gamesWon++;
            user.hoursPlayed += hoursAdded;

            double currentScore = user.performanceScore;
            double newAverage;

            if (oldGamesCount == 0) {
                newAverage = matchScore;
            }
            else {
                newAverage = ((currentScore * oldGamesCount) + matchScore) / user.gamesPlayed;
            }
            
            user.performanceScore = newAverage;

            DBManager::updateUser(user);
        }
    }
}

void RouteHandler::setupRoutes(crow::SimpleApp& app) {
    // Core Routes
    CROW_ROUTE(app, "/status")(RouteHandler::handleStatus);
    CROW_ROUTE(app, "/register").methods("POST"_method)(RouteHandler::handleRegister);
    CROW_ROUTE(app, "/login").methods("POST"_method)(RouteHandler::handleLogin);
    CROW_ROUTE(app, "/users")(RouteHandler::handleGetUsers);

    // Lobby and Matchmaking Routes
    CROW_ROUTE(app, "/joinLobby").methods("POST"_method)(RouteHandler::handleJoinLobby);
    CROW_ROUTE(app, "/create_game").methods("POST"_method)(RouteHandler::handleCreateGame);
    CROW_ROUTE(app, "/joinByPin").methods("POST"_method)(RouteHandler::handleJoinByPin);
    CROW_ROUTE(app, "/lobbyStatus")(RouteHandler::handleLobbyStatus);
    CROW_ROUTE(app, "/startGame").methods("POST"_method)(RouteHandler::handleStartGame);
    CROW_ROUTE(app, "/games/<int>/leaveGame").methods("POST"_method)(RouteHandler::handleLeaveGameWithId);

    // Game Session Routes
    CROW_ROUTE(app, "/games/<int>/state")(RouteHandler::handleGetGameState);
    CROW_ROUTE(app, "/games/<int>/playCard").methods("POST"_method)(RouteHandler::handlePlayCard);
    CROW_ROUTE(app, "/games/<int>/endTurn").methods("POST"_method)(RouteHandler::handleEndTurn);
    CROW_ROUTE(app, "/games/<int>/messages")(RouteHandler::handleGetMessages);
    CROW_ROUTE(app, "/games/<int>/sendMessage").methods("POST"_method)(RouteHandler::handleSendMessage);

    CROW_ROUTE(app, "/games/<int>/setMode").methods("POST"_method)(RouteHandler::handleSetGameMode);
}

crow::response RouteHandler::handleStatus() {
    crow::json::wvalue res;
    res["status"] = "ok";
    return crow::response(200, res.dump());
}
crow::response RouteHandler::handleLeaveGameWithId(const crow::request& req, int gameId) {
    try {
        auto body = json::parse(req.body);
        std::string username = body.at("username").get<std::string>();

        if (GameLobby::instance().leaveGame(gameId, username)) {
            return crow::response(200, R"({"success":true})");
        }
        return crow::response(409, R"({"success":false,"error":"Player not in game or game not found"})");
    }
    catch (...) {
        return crow::response(400, R"({"success":false,"error":"Invalid request"})");
    }
}
crow::response RouteHandler::handleRegister(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        if (!body.contains("username") || !body.contains("password")) {
            return crow::response(400, "Missing username or password");
        }
        std::string username = body["username"];
        std::string password = body["password"];

        if (username.empty() || username.size() > Config::MAX_USERNAME_LENGTH) return crow::response(400, "Invalid username length");

        static const std::regex usernameRegex("^[a-zA-Z0-9_]+$");
        if (!std::regex_match(username, usernameRegex)) {
            return crow::response(400, "Username contains invalid characters");
        }

        static const std::regex passwordRegex("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d@$!%*?&_]{5,}$");
        if (!std::regex_match(password, passwordRegex)) {
            return crow::response(400, "Password weak: Min 5 chars, at least 1 letter and 1 number");
        }

        if (DBManager::userExists(username)) {
            return crow::response(409, "Username already exists");
        }

        auto [hash, salt] = PasswordService::hashPasswordWithSalt(password);
        if (!DBManager::addUser(username, hash, salt)) {
            return crow::response(500, "Database Error: Could not create user");
        }

        crow::json::wvalue res; 
        res["success"] = true;
        res["message"] = "User registered successfully";
        return crow::response(201, res.dump());
    }
    catch (...) { return crow::response(400, "JSON Error"); }
}

crow::response RouteHandler::handleLogin(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        if (!body.contains("username") || !body.contains("password")) return crow::response(400);

        std::string username = body["username"];
        std::string password = body["password"];

        auto userOpt = DBManager::getUser(username);
        if (!userOpt.has_value() || !PasswordService::verifyPassword(password, userOpt->passwordHash, userOpt->salt)) {
            return crow::response(401, "Invalid credentials");
        }

        crow::json::wvalue res; res["success"] = true; res["username"] = username;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400, "JSON Error"); }
}

crow::response RouteHandler::handleCreateGame(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string pin = body["pin"];

        if (!pin.empty()) {
            static const std::regex pinRegex("^[0-9]{4}$");
            if (!std::regex_match(pin, pinRegex)) {
                return crow::response(400, "PIN must be exactly 4 digits");
            }
        }

        bool isPublic = pin.empty();

        auto gameId = GameLobby::instance().createGame(username, pin, isPublic);
        if (!gameId) return crow::response(500);

        crow::json::wvalue res; res["success"] = true; res["gameId"] = *gameId;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleGetUsers() {
    crow::json::wvalue res;
    auto users = DBManager::getAllUsers();
    auto& arr = res["users"];
    for (size_t i = 0; i < users.size(); i++) {
        arr[i]["username"] = users[i].username;
        arr[i]["games_played"] = users[i].gamesPlayed;
        arr[i]["games_won"] = users[i].gamesWon;
    }
    return crow::response(200, res.dump());
}

crow::response RouteHandler::handleJoinLobby(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string username = body["username"];

        auto gameIdOpt = GameLobby::instance().findRandomGame();
        if (gameIdOpt.has_value()) {
            if (GameLobby::instance().joinGame(*gameIdOpt, username)) {
                crow::json::wvalue res; res["success"] = true; res["gameId"] = *gameIdOpt;
                return crow::response(200, res.dump());
            }
        }

        auto newGameId = GameLobby::instance().createGame(username);
        crow::json::wvalue res; res["success"] = true; res["gameId"] = *newGameId;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleLobbyStatus() {
    crow::json::wvalue res;
    auto sessions = GameLobby::instance().getAllSessions();
    int i = 0;
    for (auto& session : sessions) {
        if (!session) continue;
        std::lock_guard lock(session->mtx);
        res["games"][i]["gameId"] = session->id;
        res["games"][i]["playersCount"] = session->players.size();
        res["games"][i]["started"] = session->started;
        i++;
    }
    return crow::response(200, res.dump());
}

crow::response RouteHandler::handleJoinByPin(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string pin = body["pin"];

        static const std::regex pinRegex("^[0-9]{4}$");
        if (!std::regex_match(pin, pinRegex)) {
            return crow::response(400, "Invalid PIN format (must be 4 digits)");
        }

        auto gameIdOpt = GameLobby::instance().findGame(pin);
        if (!gameIdOpt.has_value()) return crow::response(404, "Game not found");

        if (!GameLobby::instance().joinGame(*gameIdOpt, username)) return crow::response(409, "Full or started");

        crow::json::wvalue res; res["success"] = true; res["gameId"] = *gameIdOpt;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleStartGame(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        int gameId = body["gameId"].get<int>();
        GameLobby::instance().startGame(gameId);
        crow::json::wvalue res; res["success"] = true;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleLeaveGame(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        int gameId = body["gameId"].get<int>();
        std::string username = body["username"];

        if (GameLobby::instance().leaveGame(gameId, username)) {
            crow::json::wvalue res; res["success"] = true;
            return crow::response(200, res.dump());
        }
        return crow::response(409, "Failed to leave");
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleGetGameState(const crow::request& req, int gameId) {
    auto session = GameLobby::instance().getSession(gameId);
    if (!session) return crow::response(404);

    std::lock_guard lock(session->mtx);
    if (!session->started || !session->gameLogic) {
        nlohmann::json j;
        j["status"] = "Lobby";
        j["playerHand"] = nlohmann::json::array();
        j["stacks"] = nlohmann::json::array();
        j["isMyTurn"] = false;
        return crow::response(200, j.dump());
    }

    std::string username = req.url_params.get("username") ? req.url_params.get("username") : "";
    int pIndex = getPlayerIndex(session->players, username);
    if (pIndex == -1) return crow::response(403);

    json fullState = session->gameLogic->GetStateForClient(pIndex);

    json simpleState;
    simpleState["stacks"] = json::array();
    for (const auto& s : fullState["stacks"]) simpleState["stacks"].push_back(s["topValue"]);

    simpleState["playerHand"] = json::array();
    if (fullState["players"][pIndex].contains("hand")) {
        for (const auto& c : fullState["players"][pIndex]["hand"]) {
            simpleState["playerHand"].push_back(c["value"]);
        }
    }

    simpleState["isMyTurn"] = (fullState["currentPlayerIndex"] == pIndex && fullState["status"] == "Running");
    simpleState["status"] = fullState["status"];
    simpleState["deckCount"] = fullState["deckCount"];

    if (fullState.contains("fireMode")) {
        simpleState["fireMode"] = fullState["fireMode"];
    }

    return crow::response(200, simpleState.dump());
}

crow::response RouteHandler::handlePlayCard(const crow::request& req, int gameId) {
    try {
        auto body = json::parse(req.body);
        std::string username = body["username"];
        int cardVal = body["cardValue"];
        int stackIdx = body["stackIndex"];

        auto session = GameLobby::instance().getSession(gameId);
        if (!session) return crow::response(404);

        std::lock_guard lock(session->mtx);
        if (!session->started) return crow::response(400, "Not started");

        int pIndex = getPlayerIndex(session->players, username);
        if (pIndex == -1) return crow::response(403);

        const auto& hand = session->gameLogic->GetPlayerHand(pIndex);
        int handIndex = -1;
        for (size_t i = 0; i < hand.size(); ++i) {
            if (hand[i].GetValue() == cardVal) { handIndex = i; break; }
        }
        if (handIndex == -1) return crow::response(200, R"({"success":false,"error":"Card not in hand"})");

        bool ok = session->gameLogic->ProcessPlayCard(pIndex, handIndex, stackIdx);

        if (ok) {
            json state = session->gameLogic->GetStateForClient(pIndex);
            std::string status = state["status"];
            if (status == "Won" || status == "Lost") {
                updateStatsAfterGame(session.get(), status);
                session->started = false; 
            }
        }

        crow::json::wvalue res; res["success"] = ok;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleEndTurn(const crow::request& req, int gameId) {
    try {
        auto body = json::parse(req.body);
        std::string username = body["username"];

        auto session = GameLobby::instance().getSession(gameId);
        if (!session) return crow::response(404);

        std::lock_guard lock(session->mtx);
        int pIndex = getPlayerIndex(session->players, username);
        if (pIndex == -1) return crow::response(403);

        bool ok = session->gameLogic->ProcessEndTurn(pIndex);

        if (ok) {
            json state = session->gameLogic->GetStateForClient(pIndex);
            std::string status = state["status"];
            if (status == "Won" || status == "Lost") {
                updateStatsAfterGame(session.get(), status);
                session->started = false;
            }
        }

        crow::json::wvalue res; res["success"] = ok;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleGetMessages(const crow::request& req, int gameId) {
    auto session = GameLobby::instance().getSession(gameId);
    if (!session) return crow::response(404);

    std::lock_guard lock(session->mtx);
    crow::json::wvalue res;
    res["messages"] = session->getMessages();
    return crow::response(200, res.dump());
}

crow::response RouteHandler::handleSendMessage(const crow::request& req, int gameId) {
    try {
        auto body = json::parse(req.body);
        std::string msg = body["message"];
        std::string user = body["username"];

        auto session = GameLobby::instance().getSession(gameId);
        if (!session) return crow::response(404);

        std::lock_guard lock(session->mtx);

        std::regex filterPattern(
            R"((\b\d+\b)|(\b(unu|doi|trei|patru|cinci|sase|sapte|opt|noua|zece|unspre|doispre|treispre|paispre|cincispre|saispre|saptespre|optspre|nouaspre|douaz|treiz|patruz|cinciz|saiz|saptez|optz|nouaz|suta)\w*))",
            std::regex_constants::icase | std::regex_constants::optimize
        );

        std::string cleanMsg = std::regex_replace(msg, filterPattern, "[***]");

        session->addMessage(user + ": " + cleanMsg);

        crow::json::wvalue res; res["success"] = true;
        return crow::response(200, res.dump());
    }
    catch (...) { return crow::response(400); }
}

crow::response RouteHandler::handleSetGameMode(const crow::request& req, int gameId) {
    try {
        auto body = json::parse(req.body);
        bool fireMode = body.value("fireMode", false);

        auto session = GameLobby::instance().getSession(gameId);
        if (!session) return crow::response(404, "Game session not found");

        std::lock_guard lock(session->mtx);

        session->setGameMode(fireMode);

        crow::json::wvalue res;
        res["success"] = true;
        res["message"] = fireMode ? "Fire Mode Activated" : "Normal Mode Activated";

        return crow::response(200, res.dump());
    }
    catch (...) {
        return crow::response(400, "Invalid JSON or Request");
    }
}