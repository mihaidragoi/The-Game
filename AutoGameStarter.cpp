#include "AutoGameStarter.h"
#include "Config.h"
#include <iostream>

AutoGameStarter::AutoGameStarter() {
    monitorThread = std::thread([this]() {
        std::cout << "[AutoStarter] Background thread started.\n";
        while (!shouldStop) {
            std::this_thread::sleep_for(std::chrono::seconds(Config::AUTOSTART_CHECK_INTERVAL_SECONDS));

            auto& lobby = GameLobby::instance();
            auto gameIds = lobby.getGamesToStartAutomatically(); 

            for (int id : gameIds) {
                lobby.startGame(id); 
            }
        }
        std::cout << "[AutoStarter] Background thread stopped.\n";
        });
}

AutoGameStarter::~AutoGameStarter() {
    shouldStop = true;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}