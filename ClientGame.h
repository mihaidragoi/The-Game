#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "Card.h"
#include "Stack.h"

class ClientGame
{
public:
    ClientGame();

    void UpdateState(const nlohmann::json& j);

    bool IsMyTurn() const;

    bool CanPlayCardLocally(int handIndex, int stackIndex) const;

    const std::vector<Card>& GetMyHand() const;
    const std::vector<Stack>& GetStacks() const;
    int GetDeckCount() const;
    int GetCurrentPlayerIndex() const;
    int GetMyPlayerIndex() const;
    std::string GetGameStatus() const;
    int GetCardsPlayedThisTurn() const;

    bool IsFireMode() const;

private:
    std::string m_gameStatus;
    int m_currentPlayerIndex;
    int m_cardsPlayedThisTurn;
    int m_myPlayerIndex;
    int m_deckCount;

    bool m_fireMode;

    std::vector<Stack> m_localStacks;
    std::vector<Card> m_myHand;
};