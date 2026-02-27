#pragma once

#include <QMainWindow>
#include <memory>
#include "ui_MainGameWindow.h"

class MainGameWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainGameWindow(const std::string& username, QWidget* parent = nullptr);
private:
	std::unique_ptr<Ui::MainGameWindowClass> m_main_game_window_ui;
	std::string m_currentUsername;
private slots:
	void On_logoutButton_clicked();
	void On_account_clicked();
	void On_createGame_clicked();
	void On_findGame_clicked();
	void On_randomGame_clicked();
};

