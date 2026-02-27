#include "pch.h"
#include "Card.h"

Card::Card(int value) : m_value(value) {}

int Card::GetValue() const {
    return m_value;
}

nlohmann::json Card::ToJson() const {
    return { {"value", m_value} };
}