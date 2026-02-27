#include "LoginWindow.h"
#include <QMessageBox>
#include "MainGameWindow.h"
#include "NetworkClient.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <regex>

LoginWindow::LoginWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_login_window_ui.setupUi(this);

	connect(m_login_window_ui.loginButton, &QPushButton::clicked, this, &LoginWindow::On_loginButton_clicked);
	connect(m_login_window_ui.registerButton, &QPushButton::clicked, this, &LoginWindow::On_registerButton_clicked);
	connect(m_login_window_ui.exitButton, &QPushButton::clicked, this, &LoginWindow::On_exitButton_clicked);
	connect(m_login_window_ui.lineEditUsername, &QLineEdit::returnPressed, this, &LoginWindow::On_lineEditUsername_returnPressed);
	connect(m_login_window_ui.lineEditPassword, &QLineEdit::returnPressed, this, &LoginWindow::On_lineEditPassword_returnPressed);

	m_login_window_ui.lineEditUsername->setFocus();
}

LoginWindow::~LoginWindow()
{}

void LoginWindow::On_loginButton_clicked()
{
	QString username = m_login_window_ui.lineEditUsername->text().trimmed();
	QString password = m_login_window_ui.lineEditPassword->text().trimmed();

	std::cout << "Login attempt - User: '" << username.toStdString() << "', Pass length: " << password.size() << std::endl;

	std::string usernameStr = username.toStdString();
	if (NetworkClient::LoginUser(usernameStr, password.toStdString())) {
		QMessageBox::information(this, "Success", "Login successful!");
		auto* mainGameWindow = new MainGameWindow(usernameStr, nullptr);
		mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
		mainGameWindow->show();
		this->close();
	}
	else {
		QMessageBox::critical(this, "Error", "Invalid username or password!");
	}
}

void LoginWindow::On_registerButton_clicked()
{
	QString username = m_login_window_ui.lineEditUsername->text().trimmed();
	QString password = m_login_window_ui.lineEditPassword->text().trimmed();

	if (username.isEmpty() || password.isEmpty()) {
		QMessageBox::warning(this, "Error", "Username and password are required!");
		return;
	}

	std::cout << "Client sending username: '" << username.toStdString() << "'" << std::endl;

	if (NetworkClient::RegisterUser(username.toStdString(), password.toStdString())) {
		QMessageBox::information(this, "Success", "Registration successful!");
	}
	else {
		QMessageBox::critical(this, "Error", "Registration failed. Username may already exist.");
	}
}
void LoginWindow::On_lineEditUsername_returnPressed()
{
	On_loginButton_clicked();
}

void LoginWindow::On_lineEditPassword_returnPressed()
{
	On_loginButton_clicked();
}

void LoginWindow::On_exitButton_clicked()
{
	close();
}

bool LoginWindow::CheckCredentials(const QString& username, const QString& password)
{
	if(username.contains(' ') || username.isEmpty())
		return false;

	bool hasUpper = false;
	bool hasLower = false;
	bool hasDigit = false;
	for (const QChar c : password)
	{
		if (c.isUpper())
			hasUpper = true;
		else if (c.isLower())
			hasLower = true;
		else if (c.isDigit())
			hasDigit = true;
		else if (c.isSpace())
			return false;
	}

	return hasDigit && hasLower && hasUpper;
}

