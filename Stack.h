#pragma once
#include "Card.h"
#include <vector>
#include <nlohmann/json.hpp>
#include <optional>

enum class StackType {
    Ascending,
    Descending
};

class Stack {
private:
    StackType m_type;
    std::vector<Card> m_cards;
    int m_baseValue;
    std::optional<int> m_fireTarget;

    static constexpr int BACKWARDS_TRICK_DIFF = 10;

public:
    Stack(StackType type, int baseValue);

    void PlaceCard(Card&& card);

    int GetTopValue() const;
    bool CanPlay(const Card& card) const;
    StackType GetType() const;
    const std::vector<Card>& GetCards() const;

    void SetFireTarget(std::optional<int> target);
    std::optional<int> GetFireTarget() const;

    nlohmann::json ToJson() const;
};