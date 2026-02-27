#pragma once

#include <nlohmann/json.hpp>
#include <compare>

class Card {
private:
    int m_value;
public:
    explicit Card(int value);
    int GetValue() const;
    auto operator<=> (const Card& other) const = default;
    nlohmann::json ToJson() const;
};

