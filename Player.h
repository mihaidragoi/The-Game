#pragma once

#include <string>
#include <vector>

#include "Card.h" 
#include <nlohmann/json.hpp> 

class Player
{
public:
    explicit Player(std::string username);

    void AddCardToHand(Card&& card);


    Card PlayCard(int handIndex);

    bool IsHandEmpty() const;
    const std::string& GetUsername() const;
    const std::vector<Card>& GetHand() const;

    nlohmann::json ToJson(bool showHandContent = false) const;

private:
    std::string m_username;
    std::vector<Card> m_hand;
};