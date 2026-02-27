#pragma once

#include <QMainWindow>
#include "ui_LoginWindow.h"
#include <memory>

class MainGameWindow;

class LoginWindow : public QMainWindow
{
	Q_OBJECT

public:
	LoginWindow(QWidget *parent = nullptr);
	~LoginWindow();

private slots:
	void On_loginButton_clicked();
	void On_registerButton_clicked();
	void On_lineEditUsername_returnPressed();
	void On_lineEditPassword_returnPressed();
	void On_exitButton_clicked();

private:
	bool CheckCredentials(const QString& username, const QString& password);
	bool ActionProgress = false;
	Ui::LoginWindowClass m_login_window_ui;
};

