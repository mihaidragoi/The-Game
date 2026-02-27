#pragma once
#include <cstdint>

namespace Config {
    inline constexpr uint16_t PORT = 18080;

    inline constexpr int MAX_PLAYERS = 5;
    inline constexpr int MIN_PLAYERS_AUTOSTART = 2;

    inline constexpr int AUTOSTART_CHECK_INTERVAL_SECONDS = 5;
    inline constexpr int MAX_WAIT_SECONDS = 30;

    inline constexpr int MAX_USERNAME_LENGTH = 20;
}