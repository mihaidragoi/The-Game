#include "ClientGame.h"
#include <iostream>
#include <exception>
#include "../GameCommon/GameStatusConstants.h"

ClientGame::ClientGame()
    : m_deckCount(0)
    , m_currentPlayerIndex(-1)
    , m_myPlayerIndex(-1)
    , m_cardsPlayedThisTurn(0)
    , m_fireMode(false) 
{
}

void ClientGame::UpdateState(const nlohmann::json& j)
{
    try {
        if (j.contains("status")) {
            m_gameStatus = j.at("status").get<std::string>();
        }
        if (j.contains("currentPlayerIndex")) {
            m_currentPlayerIndex = j.at("currentPlayerIndex").get<int>();
        }
        if (j.contains("cardsPlayedThisTurn")) {
            m_cardsPlayedThisTurn = j.at("cardsPlayedThisTurn").get<int>();
        }
        if (j.contains("yourIndex")) {
            m_myPlayerIndex = j.at("yourIndex").get<int>();
        }
        if (j.contains("deckCount")) {
            m_deckCount = j.at("deckCount").get<int>();
        }
        if (j.contains("fireMode")) {
            m_fireMode = j.at("fireMode").get<bool>();
        }

        m_localStacks.clear();
        if (j.contains("stacks")) {
            const auto& stacksArray = j.at("stacks");
            m_localStacks.reserve(stacksArray.size());

            for (const auto& sJson : stacksArray) {

                int topVal = 1;
                if (sJson.is_object() && sJson.contains("topValue")) {
                    topVal = sJson.at("topValue").get<int>();
                }
                else if (sJson.is_number()) {
                    topVal = sJson.get<int>();
                }

                size_t currentIndex = m_localStacks.size();
                StackType type = (currentIndex < 2) ? StackType::Ascending : StackType::Descending;
                int baseVal = (type == StackType::Ascending) ? 1 : 100;

                Stack tempStack(type, baseVal);

                if (topVal != baseVal) {
                    tempStack.PlaceCard(Card(topVal));
                }
                m_localStacks.push_back(std::move(tempStack));
            }
        }

        m_myHand.clear();

        if (j.contains("playerHand")) {
            const auto& handArray = j.at("playerHand");
            for (const auto& val : handArray) {
                if (val.is_number()) {
                    m_myHand.emplace_back(val.get<int>());
                }
            }
        }
        else if (j.contains("players")) {
            const auto& playersArray = j.at("players");
            if (m_myPlayerIndex >= 0 && m_myPlayerIndex < static_cast<int>(playersArray.size())) {
                const auto& myData = playersArray[m_myPlayerIndex];
                if (myData.contains("hand")) {
                    const auto& handArray = myData.at("hand");
                    for (const auto& cJson : handArray) {
                        int val = cJson.at("value").get<int>();
                        m_myHand.emplace_back(val);
                    }
                }
            }
        }

    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "[ClientGame Error] JSON Parsing failed: " << e.what() << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << "[ClientGame Error] Standard exception: " << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "[ClientGame Error] Unknown error occurred during UpdateState.\n";
    }
}

bool ClientGame::IsMyTurn() const {
    return (m_currentPlayerIndex == m_myPlayerIndex) &&
        (m_gameStatus == GameStatusConstants::STATUS_RUNNING);
}

bool ClientGame::CanPlayCardLocally(int handIndex, int stackIndex) const {
    if (!IsMyTurn()) return false;

    if (handIndex < 0 || handIndex >= static_cast<int>(m_myHand.size())) return false;
    if (stackIndex < 0 || stackIndex >= static_cast<int>(m_localStacks.size())) return false;

    return m_localStacks[stackIndex].CanPlay(m_myHand[handIndex]);
}

const std::vector<Card>& ClientGame::GetMyHand() const {
    return m_myHand;
}

const std::vector<Stack>& ClientGame::GetStacks() const {
    return m_localStacks;
}

int ClientGame::GetDeckCount() const {
    return m_deckCount;
}

int ClientGame::GetCurrentPlayerIndex() const {
    return m_currentPlayerIndex;
}

int ClientGame::GetMyPlayerIndex() const {
    return m_myPlayerIndex;
}

std::string ClientGame::GetGameStatus() const {
    return m_gameStatus;
}

int ClientGame::GetCardsPlayedThisTurn() const {
    return m_cardsPlayedThisTurn;
}

bool ClientGame::IsFireMode() const {
    return m_fireMode;
}