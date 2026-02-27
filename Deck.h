#pragma once
#include "Card.h"
#include <vector>
#include <random>

class Deck {
public:
    Deck();
    void Initialize();
    void Shuffle();

    Card Draw();
    bool IsEmpty() const;
    int GetCount() const;

    nlohmann::json ToJson() const;

private:
    std::vector<Card> m_cards;
    std::mt19937 m_rng;
};