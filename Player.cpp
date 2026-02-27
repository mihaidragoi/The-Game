#include "Player.h"
#include <stdexcept>
#include <utility> 

Player::Player(std::string username)
    : m_username(std::move(username)) 
{
}

void Player::AddCardToHand(Card&& card)
{
    m_hand.push_back(std::move(card));
}

Card Player::PlayCard(int handIndex)
{
    Card cardToPlay = std::move(m_hand[handIndex]);

    m_hand.erase(m_hand.begin() + handIndex);

    return cardToPlay;
}

bool Player::IsHandEmpty() const
{
    return m_hand.empty();
}

const std::string& Player::GetUsername() const
{
    return m_username;
}

const std::vector<Card>& Player::GetHand() const
{
    return m_hand;
}

nlohmann::json Player::ToJson(bool showHandContent) const {
    nlohmann::json j;
    j["username"] = m_username;
    j["cardCount"] = m_hand.size();

    if (showHandContent) {
        j["hand"] = nlohmann::json::array();
        for (const auto& card : m_hand) {
            j["hand"].push_back(card.ToJson());
        }
    }
    return j;
}