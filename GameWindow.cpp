#include "GameWindow.h"
#include "NetworkClient.h"
#include "MainGameWindow.h"
#include <QMessageBox>
#include <vector>
#include <algorithm>
#include <qfile.h>
#include <format>

GameWindow::GameWindow(MainGameWindow* mainWindow, const std::string& username, const std::string& gameId, QWidget* parent)
    : QMainWindow(parent), m_mainWindow(mainWindow), m_username(username), m_currentGameId(gameId)
{
    m_game_window_ui.setupUi(this);
    connect(m_game_window_ui.m_leaveGame, &QPushButton::clicked, this, &GameWindow::On_leaveGameButton_clicked);
    connect(m_game_window_ui.m_endTurnButton, &QPushButton::clicked, this, &GameWindow::On_EndTurn_Clicked);
    connect(m_game_window_ui.m_startGameButton, &QPushButton::clicked, this, &GameWindow::On_StartGame_Clicked);
    connect(m_game_window_ui.sendButton, &QPushButton::clicked, this, &GameWindow::On_sendButton_clicked);
	connect(m_game_window_ui.m_normalMode, &QPushButton::clicked, this, &GameWindow::On_normalMode_clicked);
	connect(m_game_window_ui.m_fireMode, &QPushButton::clicked, this, &GameWindow::On_fireMode_clicked);

    std::vector<QPushButton*> potentialButtons = {
        m_game_window_ui.card1, m_game_window_ui.card2, m_game_window_ui.card3, m_game_window_ui.card4,
        m_game_window_ui.card5, m_game_window_ui.card6, m_game_window_ui.card7, m_game_window_ui.card8
    };

    m_handButtons.clear();
    for (auto* btn : potentialButtons) {
        if (btn) {
            m_handButtons.push_back(btn);
            connect(btn, &QPushButton::clicked, this, &GameWindow::On_HandCard_Clicked);
        }
    }

    if (m_game_window_ui.deckAscending1) {
        m_game_window_ui.deckAscending1->setProperty("stackIndex", 0);
        connect(m_game_window_ui.deckAscending1, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckAscending2) {
        m_game_window_ui.deckAscending2->setProperty("stackIndex", 1);
        connect(m_game_window_ui.deckAscending2, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckDescending1) {
        m_game_window_ui.deckDescending1->setProperty("stackIndex", 2);
        connect(m_game_window_ui.deckDescending1, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckDescending2) {
        m_game_window_ui.deckDescending2->setProperty("stackIndex", 3);
        connect(m_game_window_ui.deckDescending2, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }

    m_chatUpdateTimer = new QTimer(this);
    connect(m_chatUpdateTimer, &QTimer::timeout, this, &GameWindow::UpdateGameState);
    m_chatUpdateTimer->start(1000);

    UpdateGameState();
    UpdateChatMessages();
}

GameWindow::~GameWindow()
{
}

void GameWindow::On_StartGame_Clicked()
{
    bool success = NetworkClient::StartGame(m_currentGameId);
    if (success) {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->hide();
        if (m_game_window_ui.m_endTurnButton) m_game_window_ui.m_endTurnButton->show();
        QMessageBox::information(this, "Info", "Game Started!");
        UpdateGameState();
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to start game (Maybe already started?)");
    }
}

void GameWindow::On_EndTurn_Clicked()
{
   
    if (NetworkClient::EndTurn(m_currentGameId, m_username)) {
        UpdateGameState();
        m_selectedCardValue = -1;
    }
    else {
        QMessageBox::warning(this, "Action Failed", "Cannot end turn yet.");
    }
}

void GameWindow::UpdateChatMessages()
{
    if (!m_game_window_ui.messages) return;

    std::vector<std::string> messages = NetworkClient::GetMessages(m_currentGameId);
    if (messages.empty()) return;

    auto lastMessage = messages | std::views::drop(m_game_window_ui.messages->count());

    if (messages.size() > (size_t)m_game_window_ui.messages->count())
    {
        std::ranges::for_each(lastMessage, [this](const std::string& msg)
            {
                m_game_window_ui.messages->addItem(QString::fromStdString(msg));
            });

        m_game_window_ui.messages->scrollToBottom();
    }
}

void GameWindow::On_HandCard_Clicked()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    QVariant val = clickedButton->property("cardValue");
    if (val.isValid()) m_selectedCardValue = val.toInt();

    std::ranges::for_each(m_handButtons, [](QPushButton* btn) {
        if (btn) btn->setStyleSheet(btn->styleSheet().remove("border: 2px solid pink;"));
        });

    clickedButton->setStyleSheet(clickedButton->styleSheet() + "border: 2px solid pink;");
}

void GameWindow::On_Stack_Clicked()
{
    if (m_selectedCardValue == -1) return;
    QPushButton* stackButton = qobject_cast<QPushButton*>(sender());
    if (!stackButton) return;

    int stackIndex = stackButton->property("stackIndex").toInt();
    
    bool isSucces = NetworkClient::PlayCard(m_currentGameId, m_selectedCardValue, stackIndex, m_username);
    if (isSucces)
    {
        m_selectedCardValue = -1;
        UpdateGameState();
        for (auto btn : m_handButtons) {
            if (btn) btn->setStyleSheet("");
        }
    }
}

void GameWindow::UpdateGameState()
{
   
    auto gameState = NetworkClient::GetGameState(m_currentGameId, m_username);

    m_isOnFire = gameState.m_fireMode;
    UpdateGameStyle();

    if (gameState.m_status == "Won") {
        if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
        QMessageBox::information(this, "Game Over", "You Won!");

        auto* mainGameWindow = new MainGameWindow(nullptr);
        mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainGameWindow->show();
        this->close();
        return;
    }
    if (gameState.m_status == "Lost") {
        if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
        QMessageBox::information(this, "Game Over", "You Lost!");

        auto* mainGameWindow = new MainGameWindow(nullptr);
        mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainGameWindow->show();
        this->close();
        return;
    }

    bool isRunning = (gameState.m_stacks.size() >= 4);

    if (isRunning) {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->hide();
        if (m_game_window_ui.m_endTurnButton) {
            m_game_window_ui.m_endTurnButton->show();
            m_game_window_ui.m_endTurnButton->setEnabled(gameState.m_isMyTurn);
        }

        m_isOnFire = gameState.m_fireMode;
        UpdateGameStyle();

        if (m_isOnFire) {
            m_game_window_ui.m_fireMode->setStyleSheet("background-color: red; color: white;");
            m_game_window_ui.m_normalMode->setStyleSheet("background-color: gray; color: white;");
        }
        else {
            m_game_window_ui.m_normalMode->setStyleSheet("background-color: green; color: white;");
            m_game_window_ui.m_fireMode->setStyleSheet("background-color: gray; color: white;");
        }

        if (gameState.m_isMyTurn) m_game_window_ui.m_info->setText("The Game - YOUR TURN");
        else m_game_window_ui.m_info->setText("The Game - Waiting for others...");

        if (m_game_window_ui.m_gameInfoLabel) {
            m_game_window_ui.m_gameInfoLabel->setText("Deck: " + QString::number(gameState.m_deckCount) + " cards");
        }

        int f1 = (gameState.m_fireTargets.size() > 0) ? gameState.m_fireTargets[0] : -1;
        int f2 = (gameState.m_fireTargets.size() > 1) ? gameState.m_fireTargets[1] : -1;
        int f3 = (gameState.m_fireTargets.size() > 2) ? gameState.m_fireTargets[2] : -1;
        int f4 = (gameState.m_fireTargets.size() > 3) ? gameState.m_fireTargets[3] : -1;

        if (m_game_window_ui.deckAscending1) {
            SetCardImage(m_game_window_ui.deckAscending1, gameState.m_stacks[0]);
        }
        if (m_game_window_ui.deckAscending2) {
            SetCardImage(m_game_window_ui.deckAscending2, gameState.m_stacks[1]);
        }
        if (m_game_window_ui.deckDescending1) {
            SetCardImage(m_game_window_ui.deckDescending1, gameState.m_stacks[2]);
        }
        if (m_game_window_ui.deckDescending2) {
            SetCardImage(m_game_window_ui.deckDescending2, gameState.m_stacks[3]);
        }

        UpdateHand(gameState.m_playerHand);
    }
    else {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->show();
        if (m_game_window_ui.m_endTurnButton) m_game_window_ui.m_endTurnButton->hide();
        this->setWindowTitle("The Game - Playing time");
        if (m_game_window_ui.m_gameInfoLabel) m_game_window_ui.m_gameInfoLabel->setText("Waiting to start...");
    }

    if (gameState.m_isMyTurn) {
        if (m_isOnFire) {
            m_game_window_ui.m_info->setText("The Game - YOUR TURN 🔥 (FIRE MODE) 🔥");
        }
        else {
            m_game_window_ui.m_info->setText("The Game - YOUR TURN");
        }
    }
    else {
        if (m_isOnFire) {
            m_game_window_ui.m_info->setText("The Game - Waiting for others... 🔥 (FIRE MODE) 🔥");
        }
        else {
            m_game_window_ui.m_info->setText("The Game - Waiting for others...");
        }
    }

    UpdateChatMessages();
}

void GameWindow::UpdateHand(const std::vector<int>& cardsReceived)
{
    size_t safeLimit = (std::min)(static_cast<size_t>(cardsReceived.size()), static_cast<size_t>(m_handButtons.size()));

	auto activeButtons = m_handButtons | std::views::take(safeLimit);
	int cardIndex = 0;

    std::ranges::for_each(activeButtons, [this, &cardsReceived, &cardIndex](QPushButton* button) {
		if (!button) return;
		int cardValue = cardsReceived[cardIndex++];
		std::string imagePath = std::format(":/cards/GameImages/GameCards/ResizedCards/GameCard{}.jpg", cardValue);
		std::string style = std::format("border-image: url(\"{}\"); background : transparent;", imagePath);
		button->setProperty("cardValue", cardValue);
		button->setText("");
		button->setStyleSheet(QString::fromStdString(style));
		button->setEnabled(true);
		button->show();
		});

	auto inactiveButtons = m_handButtons | std::views::drop(safeLimit);
    std::ranges::for_each(inactiveButtons, [](QPushButton* button) {
        if (!button) return;
        button->setProperty("cardValue", -1);
        button->setText("");
        button->setEnabled(false);
        button->hide();
        });
}

void GameWindow::SetCardImage(QPushButton* button, int cardValue)
{
    if (!button || cardValue < 0) return;

    std::string imagePath = std::format(":/cards/GameImages/GameCards/ResizedCards/GameCard{}.jpg", cardValue);
    std::string imagePathJpeg = std::format(":/cards/GameImages/GameCards/ResizedCards/GameCard{}.jpeg", cardValue);

    if (QFile::exists(QString::fromStdString(imagePath)))
    {
        std::string style = std::format("border-image: url(\"{}\"); background : transparent;", imagePath);
        button->setStyleSheet(QString::fromStdString(style));
        button->setText("");
    }
    else if (QFile::exists(QString::fromStdString(imagePathJpeg)))
    {
        std::string style = std::format("border-image: url(\"{}\"); background : transparent;", imagePathJpeg);
        button->setStyleSheet(QString::fromStdString(style));
        button->setText("");
    }
    else
    {
        button->setText(QString::number(cardValue));
    }
}

void GameWindow::On_leaveGameButton_clicked()
{
    auto confirm = QMessageBox::question(this, "Leave Game", "Are you sure you want to leave the game?");
    if (confirm == QMessageBox::Yes)
    {
        bool success = NetworkClient::LeaveGame(m_currentGameId, m_username);
        if (success)
        {
            if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
            m_mainWindow->show(); 
            this->close();       
        }
        else
        {
            QMessageBox::warning(this, "Error", "Failed to leave the game.");
        }
    }
}

void GameWindow::UpdateGameStyle()
{
    if (m_isOnFire)
    {
		m_game_window_ui.m_info->setStyleSheet("color: red; font-weight: bold;");
		m_game_window_ui.m_info->setText("The Game - 🔥 YOU ARE ON FIRE! 🔥");
        }
    else
    {
        m_game_window_ui.m_info->setStyleSheet("");
		m_game_window_ui.m_info->setText("The Game - Normal Mode");
    }
}

void GameWindow::On_normalMode_clicked()
{
    NetworkClient::SetGameMode(m_currentGameId, false);
    m_isOnFire = false;
	UpdateGameStyle();

	m_game_window_ui.m_normalMode->setEnabled(false);
	m_game_window_ui.m_fireMode->setEnabled(false);

    m_game_window_ui.m_normalMode->setStyleSheet("background-color: gray; color: white;");


}

void GameWindow::On_fireMode_clicked()
{
    NetworkClient::SetGameMode(m_currentGameId, true);
    m_isOnFire = true;
	UpdateGameStyle();

	m_game_window_ui.m_fireMode->setEnabled(false);
    m_game_window_ui.m_normalMode->setEnabled(false);
	m_game_window_ui.m_fireMode->setStyleSheet("background-color: red; color: white;");

}

void GameWindow::On_sendButton_clicked()
{
    if (!m_game_window_ui.messageCurrent) return;
    QString message = m_game_window_ui.messageCurrent->text();
    if (!message.isEmpty())
    {
       
        if (NetworkClient::SendMessage(m_currentGameId, message.toStdString(), m_username))
        {
            m_game_window_ui.messageCurrent->clear();
            UpdateChatMessages();
        }
    }
}

void GameWindow::SetStackVisuals(QPushButton* button, int cardValue, int fireTarget)
{
    SetCardImage(button, cardValue);

    if (fireTarget != -1) {
        QString currentStyle = button->styleSheet();
        if (!currentStyle.contains("border: 4px solid red")) {
            button->setStyleSheet(currentStyle + "border: 4px solid red; color: red; font-weight: bold; font-size: 16px;");
        }

        button->setText("🔥 NEED " + QString::number(fireTarget) + " 🔥");
    }
    else {

        QString currentStyle = button->styleSheet();
        if (currentStyle.contains("border: 4px solid red")) {

            SetCardImage(button, cardValue);
        }

        button->setText("");
    }
}