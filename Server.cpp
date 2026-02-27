#define CROW_MAIN

#include <crow.h>
#include <crow/websocket.h>
#include <crow/json.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "DBManager.h"
#include "RouteHandler.h" 
#include "Config.h"
#include "AutoGameStarter.h"
#include "GameLobby.h"

using json = nlohmann::json;

int main() {
    
    DBManager::init();
    AutoGameStarter autoStarter;
    crow::SimpleApp app;

    RouteHandler::setupRoutes(app);

    CROW_WEBSOCKET_ROUTE(app, "/game")
        .onopen([](crow::websocket::connection& conn) {
        std::cout << "[WS] Client connected\n";
            })
        .onmessage([](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
        try {
            auto msg = crow::json::load(data);
            if (!msg) return;

            if (msg.has("action") && msg["action"].s() == "join") {
                std::string username = msg["username"].s();
                int gameId = msg["gameId"].i();

                GameLobby::instance().registerWebSocket(gameId, username, &conn);

                conn.send_text(R"({"action":"joined","status":"ok"})");
            }
        }
        catch (...) {
            conn.send_text(R"({"error":"Invalid JSON"})");
        }
            })
        .onclose([](crow::websocket::connection& conn, const std::string& reason, uint16_t status) {
        std::cout << "[WS] Client disconnected: " << reason << " (Status: " << status << ")\n";
            });

    constexpr uint16_t PORT = Config::PORT;
    std::cout << std::endl;
    std::cout << " The Game server started on http://localhost:" << PORT << "/status\n";
    std::cout << " WebSocket available at ws://localhost:" << PORT << "/game\n";
    std::cout << " Press Ctrl+C to stop.\n\n";

    app.port(PORT).multithreaded().run();
    return 0;
}