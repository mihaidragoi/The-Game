#pragma once

#include "Card.h"
#include "Stack.h"
#include "Player.h"
#include "Deck.h"
#include <nlohmann/json.hpp>

#include <vector>
#include <string>
#include <memory>

class GameLogic
{
public:
    enum class GameStatus { Running, Won, Lost };

    explicit GameLogic(const std::vector<std::string>& playerNames);

    bool ProcessPlayCard(int playerIndex, int handCardIndex, int stackIndex);
    bool ProcessEndTurn(int playerIndex);

    void SetFireMode(bool enabled);
    bool IsFireMode() const;

    GameStatus GetStatus() const;
    int GetCurrentPlayerIndex() const;

    const std::vector<Card>& GetPlayerHand(int playerIndex) const;
    nlohmann::json GetStateForClient(int requestingPlayerIndex) const;

    void Initialize();

private:
    std::vector<std::unique_ptr<Player>> m_players;
    std::vector<std::unique_ptr<Stack>> m_stacks;

    Deck m_deck;
    GameStatus m_status;
    int m_currentPlayerIndex;
    int m_cardsPlayedThisTurn;
    bool m_fireMode = false;

    static constexpr int HAND_SIZE_2_PLAYERS = 8;
    static constexpr int HAND_SIZE_3_PLAYERS = 7;
    static constexpr int HAND_SIZE_DEFAULT = 6;

    void CheckGameStatus();

    bool HasAnyValidMoves() const;


};