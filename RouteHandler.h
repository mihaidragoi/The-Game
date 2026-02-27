#pragma once

#include <crow.h>

class RouteHandler {
public:
    
    static void setupRoutes(crow::SimpleApp& app);

private:

    // Internal request handlers 
    static crow::response handleStatus();
    static crow::response handleRegister(const crow::request& req);
    static crow::response handleLogin(const crow::request& req);
    static crow::response handleJoinLobby(const crow::request& req);
    static crow::response handleCreateGame(const crow::request& req);
    static crow::response handleLobbyStatus();
    static crow::response handleJoinByPin(const crow::request& req);
    static crow::response handleStartGame(const crow::request& req);
    static crow::response handleGetUsers();
    static crow::response handleLeaveGame(const crow::request& req);


    // In-game session handlers
    static crow::response handleLeaveGameWithId(const crow::request& req, int gameId);
    static crow::response handleGetGameState(const crow::request& req, int gameId);
    static crow::response handlePlayCard(const crow::request& req, int gameId);
    static crow::response handleEndTurn(const crow::request& req, int gameId);
    static crow::response handleGetMessages(const crow::request& req, int gameId);
    static crow::response handleSendMessage(const crow::request& req, int gameId);

    static crow::response handleSetGameMode(const crow::request& req, int gameId);
};