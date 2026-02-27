#include "MainGameWindow.h"
#include <qtimer.h>
#include <qdatetime.h>
#include <LoginWindow.h>
#include <AccountWindow.h>
#include <GameWindow.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include "NetworkClient.h" 

MainGameWindow::MainGameWindow(const std::string& username, QWidget* parent)
	: QMainWindow(parent),
	m_currentUsername(username), 
	m_main_game_window_ui(std::make_unique<Ui::MainGameWindowClass>())
{
	m_main_game_window_ui->setupUi(this);

	connect(m_main_game_window_ui->account, &QPushButton::clicked, this, &MainGameWindow::On_account_clicked);
	connect(m_main_game_window_ui->createGame, &QPushButton::clicked, this, &MainGameWindow::On_createGame_clicked);
	connect(m_main_game_window_ui->findGame, &QPushButton::clicked, this, &MainGameWindow::On_findGame_clicked);
	connect(m_main_game_window_ui->randomGame, &QPushButton::clicked, this, &MainGameWindow::On_randomGame_clicked);
	connect(m_main_game_window_ui->logoutButton, &QPushButton::clicked, this, &MainGameWindow::On_logoutButton_clicked);

	auto* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [this]() {
		const auto now = QDateTime::currentDateTime();
		const QString dateTimeText = now.toString("yyyy-MM-dd HH:mm:ss");
		m_main_game_window_ui->timeDate->setText(dateTimeText);
		});
	timer->start(1000);
}

void MainGameWindow::On_account_clicked()
{
	auto* accountWindow = new AccountWindow(m_currentUsername, nullptr);
	accountWindow->setAttribute(Qt::WA_DeleteOnClose);
	accountWindow->show();
	this->close();
}

void MainGameWindow::On_createGame_clicked()
{
	bool ok;
	QString pin = QInputDialog::getText(this, tr("Create Game"), tr("Enter a PIN for the game (optional):"), QLineEdit::Normal, "", &ok);
	if (ok) {
		std::string pinStd = pin.toStdString();

		int gameId = NetworkClient::CreateGame(m_currentUsername, pinStd);
		if (gameId != -1) {
			QMessageBox::information(this, tr("Game Created"), tr("Game created! ID: %1").arg(gameId));
			auto* gameWindow = new GameWindow(this, m_currentUsername, std::to_string(gameId), nullptr);
			gameWindow->setAttribute(Qt::WA_DeleteOnClose);
			gameWindow->showMaximized();
			this->hide(); 
		}
		else {
			QMessageBox::warning(this, tr("Error"), tr("Failed to create game."));
		}
	}
}

void MainGameWindow::On_findGame_clicked()
{
	bool ok;
	QString pin = QInputDialog::getText(this, tr("Find Game"), tr("Enter the PIN for the game:"), QLineEdit::Normal, "", &ok);
	if (ok && !pin.isEmpty()) {
		std::string pinStd = pin.toStdString();

		int gameId = NetworkClient::JoinGame(m_currentUsername, pinStd);
		if (gameId != -1) {
			QMessageBox::information(this, tr("Success"), tr("Joined game ID: %1").arg(gameId));
			auto* gameWindow = new GameWindow(this, m_currentUsername, std::to_string(gameId), nullptr);
			gameWindow->setAttribute(Qt::WA_DeleteOnClose);
			gameWindow->showMaximized();
			this->hide();
		}
		else {
			QMessageBox::warning(this, tr("Error"), tr("Game not found or full."));
		}
	}
}

void MainGameWindow::On_randomGame_clicked()
{
	QMessageBox::information(this, tr("Random Game"), tr("Searching for a random game..."));

	int gameId = NetworkClient::JoinRandomGame(m_currentUsername);
	if (gameId != -1) {
		QMessageBox::information(this, tr("Game Found"), tr("Joined game ID: %1").arg(gameId));
		auto* gameWindow = new GameWindow(this, m_currentUsername, std::to_string(gameId), nullptr);
		gameWindow->setAttribute(Qt::WA_DeleteOnClose);
		gameWindow->showMaximized();
		this->hide();
	}
	else {
		QMessageBox::warning(this, tr("Error"), tr("No public games available. Create one!"));
	}
}

void MainGameWindow::On_logoutButton_clicked()
{
	auto* loginWindow = new LoginWindow(nullptr);
	loginWindow->setAttribute(Qt::WA_DeleteOnClose);
	QMessageBox::information(this, "Logged Out", "You have been logged out successfully.");
	loginWindow->show();
	this->close();
}