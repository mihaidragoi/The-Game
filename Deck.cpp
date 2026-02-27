#include "Deck.h"
#include <algorithm>
#include <stdexcept>

Deck::Deck() : m_rng(std::random_device{}()) {}

void Deck::Initialize() {
	m_cards.clear();

	const int MIN_CARD_VALUE = 2;
	const int MAX_CARD_VALUE = 99;

	for (int value = MIN_CARD_VALUE; value <= MAX_CARD_VALUE; ++value) {
		m_cards.emplace_back(value);
	}

	Shuffle();
}
void Deck::Shuffle() {
	std::ranges::shuffle(m_cards, m_rng);
}

Card Deck::Draw() {
	Card topCard = std::move(m_cards.back());
	m_cards.pop_back();
	return topCard;
}

bool Deck::IsEmpty() const {
	return m_cards.empty();
}

int Deck::GetCount() const
{
	return m_cards.size();
}

nlohmann::json Deck::ToJson() const
{
	return {
			{"cardsLeft", GetCount()},
			{"isEmpty", IsEmpty()}
	};
}


