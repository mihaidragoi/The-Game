#pragma once
#include <thread>
#include <atomic>
#include <chrono>
#include "GameLobby.h"

class AutoGameStarter {
    std::thread monitorThread;
    std::atomic<bool> shouldStop{ false };

public:
    AutoGameStarter();
    ~AutoGameStarter();

    AutoGameStarter(const AutoGameStarter&) = delete;
    AutoGameStarter& operator=(const AutoGameStarter&) = delete;
};