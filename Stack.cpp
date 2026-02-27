#include "pch.h"
#include "Stack.h"

Stack::Stack(StackType type, int baseValue)
    : m_type{ type }, m_baseValue{ baseValue }, m_fireTarget{ std::nullopt }
{
    m_cards.reserve(30);
    m_cards.emplace_back(m_baseValue);
}

void Stack::PlaceCard(Card&& card)
{
    m_cards.push_back(std::move(card));
}

int Stack::GetTopValue() const
{
    return m_cards.back().GetValue();
}

bool Stack::CanPlay(const Card& card) const
{
    const int currentTop = GetTopValue();
    const int cardVal = card.GetValue();

    if (m_type == StackType::Ascending && (cardVal - currentTop == -BACKWARDS_TRICK_DIFF))
        return true;

    if (m_type == StackType::Descending && (cardVal - currentTop == BACKWARDS_TRICK_DIFF))
        return true;

    if (m_type == StackType::Ascending) {
        return cardVal > currentTop;
    }

    return cardVal < currentTop;
}

StackType Stack::GetType() const { return m_type; }

const std::vector<Card>& Stack::GetCards() const { return m_cards; }

void Stack::SetFireTarget(std::optional<int> target) {
    m_fireTarget = target;
}

std::optional<int> Stack::GetFireTarget() const {
    return m_fireTarget;
}

nlohmann::json Stack::ToJson() const
{
    return {
        {"type", (m_type == StackType::Ascending ? "Ascending" : "Descending")},
        {"topValue", GetTopValue()},
        {"fireTarget", m_fireTarget.value_or(-1)}
    };
}