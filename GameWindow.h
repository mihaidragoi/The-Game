#pragma once

#include <QMainWindow>
#include "ui_GameWindow.h"
#include <QTimer>
#include <QPushButton>
#include <QList>
#include <string>
#include <vector>

class MainGameWindow; 

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    
    explicit GameWindow(MainGameWindow* mainWindow, const std::string& username, const std::string& gameId, QWidget* parent = nullptr);
    ~GameWindow();

private:
    Ui::GameWindowClass m_game_window_ui;
    QTimer* m_chatUpdateTimer = nullptr;
    MainGameWindow* m_mainWindow;
    std::string m_username;
    std::string m_currentGameId;
    QList<QPushButton*> m_handButtons;
    int m_selectedCardValue = -1;
	bool m_isOnFire = false;

private slots:
    void On_sendButton_clicked();
    void UpdateChatMessages();
    void On_HandCard_Clicked();
    void On_Stack_Clicked();
    void On_EndTurn_Clicked();
    void On_StartGame_Clicked();
    void UpdateGameState();
    void UpdateHand(const std::vector<int>& cardsReceived);
    void SetCardImage(QPushButton* button, int cardValue);
    void On_leaveGameButton_clicked();
    void UpdateGameStyle();
	void On_normalMode_clicked();
	void On_fireMode_clicked();
    void SetStackVisuals(QPushButton* button, int cardValue, int fireTarget);
};