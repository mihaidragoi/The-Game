#include "GameLogic.h"
#include <algorithm>
#include <ranges>
#include "../GameCommon/GameStatusConstants.h"

GameLogic::GameLogic(const std::vector<std::string>& playerNames)
    : m_status(GameStatus::Running)
    , m_currentPlayerIndex(0)
    , m_cardsPlayedThisTurn(0)
{

    m_players.reserve(playerNames.size());
    for (const auto& name : playerNames) {
        m_players.push_back(std::make_unique<Player>(name));
    }

    m_stacks.reserve(4);
    m_stacks.push_back(std::make_unique<Stack>(StackType::Ascending, 1));
    m_stacks.push_back(std::make_unique<Stack>(StackType::Ascending, 1));
    m_stacks.push_back(std::make_unique<Stack>(StackType::Descending, 100));
    m_stacks.push_back(std::make_unique<Stack>(StackType::Descending, 100));
}

void GameLogic::Initialize() {
    m_deck.Initialize();

    m_status = GameStatus::Running;
    m_currentPlayerIndex = 0;
    m_cardsPlayedThisTurn = 0;

    int cardsPerPlayer = HAND_SIZE_DEFAULT;

    if (m_players.size() == 2) {
        cardsPerPlayer = HAND_SIZE_2_PLAYERS;
    }
    else if (m_players.size() == 3) {
        cardsPerPlayer = HAND_SIZE_3_PLAYERS;
    }

    for (auto& player : m_players) {
        for (int i = 0; i < cardsPerPlayer && !m_deck.IsEmpty(); ++i) {
            player->AddCardToHand(m_deck.Draw());
        }
    }
}

void GameLogic::SetFireMode(bool enabled) {
    m_fireMode = enabled;
}

bool GameLogic::IsFireMode() const {
    return m_fireMode;
}

bool GameLogic::ProcessPlayCard(int playerIndex, int handCardIndex, int stackIndex)
{
    if (m_status != GameStatus::Running) return false;
    if (playerIndex != m_currentPlayerIndex) return false;

    if (stackIndex < 0 || stackIndex >= static_cast<int>(m_stacks.size())) return false;

    Player& player = *m_players[playerIndex];
    const auto& hand = player.GetHand();

    if (handCardIndex < 0 || handCardIndex >= static_cast<int>(hand.size())) return false;

    const Card& cardToPlay = hand[handCardIndex];
    Stack& targetStack = *m_stacks[stackIndex];

    if (m_fireMode) {
        for (size_t i = 0; i < m_stacks.size(); ++i) {
            auto fireTarget = m_stacks[i]->GetFireTarget();
            if (fireTarget.has_value()) {
                if (i == stackIndex && cardToPlay.GetValue() == fireTarget.value()) {
                    m_stacks[i]->SetFireTarget(std::nullopt);
                    break;
                }
                else {
                    m_status = GameStatus::Lost;
                    return true; 
                }
            }
        }
    }

    if (targetStack.CanPlay(cardToPlay)) {
        Card playedCard = player.PlayCard(handCardIndex);
        int playedValue = playedCard.GetValue();
        targetStack.PlaceCard(std::move(playedCard));

        m_cardsPlayedThisTurn++;

        if (m_fireMode && (playedValue % 11 == 0) && playedValue != 0 && playedValue != 100) {
            int nextNeeded = -1;
            if (targetStack.GetType() == StackType::Ascending) {
                nextNeeded = playedValue + 1;
            }
            else {
                nextNeeded = playedValue - 1;
            }
            targetStack.SetFireTarget(nextNeeded);
        }

        CheckGameStatus();

        return true;
    }

    return false;
}

bool GameLogic::ProcessEndTurn(int playerIndex)
{
    if (m_status != GameStatus::Running) return false;
    if (playerIndex != m_currentPlayerIndex) return false;

    int minCardsToPlay = m_deck.IsEmpty() ? 1 : 2;
    if (m_players[playerIndex]->IsHandEmpty()) minCardsToPlay = 0;

    if (m_cardsPlayedThisTurn < minCardsToPlay) {
        CheckGameStatus();
        return false;
    }

    Player& player = *m_players[playerIndex];
    while (m_cardsPlayedThisTurn > 0 && !m_deck.IsEmpty()) {
        player.AddCardToHand(m_deck.Draw());
        m_cardsPlayedThisTurn--;
    }

    m_currentPlayerIndex = (m_currentPlayerIndex + 1) % static_cast<int>(m_players.size());
    m_cardsPlayedThisTurn = 0;

    CheckGameStatus();

    return true;
}

bool GameLogic::HasAnyValidMoves() const
{
    const auto& currentPlayer = *m_players[m_currentPlayerIndex];

    if (currentPlayer.GetHand().empty()) return true;

    return std::ranges::any_of(currentPlayer.GetHand(), [&](const Card& card) {
        return std::ranges::any_of(m_stacks, [&](const auto& stackPtr) {
            return stackPtr->CanPlay(card);
            });
        });
}

void GameLogic::CheckGameStatus()
{
    if (m_deck.IsEmpty()) {
        bool allHandsEmpty = std::ranges::all_of(m_players, [](const auto& p) {
            return p->IsHandEmpty();
            });

        if (allHandsEmpty) {
            m_status = GameStatus::Won;
            return;
        }
    }

    int minCardsToPlay = m_deck.IsEmpty() ? 1 : 2;

    if (m_players[m_currentPlayerIndex]->IsHandEmpty()) {
        minCardsToPlay = 0;
    }

    if (m_cardsPlayedThisTurn < minCardsToPlay && !HasAnyValidMoves()) {
        m_status = GameStatus::Lost;
    }
}


GameLogic::GameStatus GameLogic::GetStatus() const { return m_status; }
int GameLogic::GetCurrentPlayerIndex() const { return m_currentPlayerIndex; }

const std::vector<Card>& GameLogic::GetPlayerHand(int playerIndex) const {
    return m_players[playerIndex]->GetHand();
}

nlohmann::json GameLogic::GetStateForClient(int requestingPlayerIndex) const {
    nlohmann::json j;

    if (m_status == GameStatus::Running) {
        j["status"] = GameStatusConstants::STATUS_RUNNING;
    }
    else if (m_status == GameStatus::Won) {
        j["status"] = GameStatusConstants::STATUS_WON;
    }
    else {
        j["status"] = GameStatusConstants::STATUS_LOST;
    }

    j["currentPlayerIndex"] = m_currentPlayerIndex;
    j["cardsPlayedThisTurn"] = m_cardsPlayedThisTurn;
    j["yourIndex"] = requestingPlayerIndex;
    j["deckCount"] = m_deck.GetCount();

    j["stacks"] = nlohmann::json::array();
    for (const auto& stackPtr : m_stacks) {
        j["stacks"].push_back(stackPtr->ToJson());
    }

    j["players"] = nlohmann::json::array();
    for (int i = 0; i < static_cast<int>(m_players.size()); ++i) {
        bool showHandContent = (i == requestingPlayerIndex);
        j["players"].push_back(m_players[i]->ToJson(showHandContent));
    }

    j["fireMode"] = m_fireMode;

    return j;
}