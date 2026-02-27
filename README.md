The Game
A multiplayer card game based on a Client-Server architecture, developed in C++.

🚀 Main Features
Client-Server Architecture: Efficient network communication with a clear separation between server logic and user interface.

Graphical Interface (Client): Built using the Qt framework (includes Login, Account Creation, Main Menu, and Game windows).

C++ Server: Implemented with the Crow micro-framework (crow_all.h) for managing API routes and connections.

Security & Accounts: Protected authentication with OpenSSL (password hashing), integrated Rate Limiter to prevent spam/DDoS attacks, and database management (via DBManager and thegame.db).

Lobby System: Organized into sessions (GameSession) and waiting rooms (GameLobby).

📁 Project Structure
The code is divided into functional modules:

Client UI (*.ui, *Window.cpp/h): Manages the application windows for the player.

Server Logic (Server.cpp, RouteHandler, DBManager): Handles user data and incoming requests.

Core Game (GameLogic, Card, Deck, Player): The actual rules and mechanics of the card game.

Security (SecurityEngine, PasswordService): Encrypts sensitive data.

🛠️ Required Dependencies
To compile and run this project, you will need:

Visual Studio (MSVC) (the project includes .vcxproj files).

Qt Framework (to run the graphical client).

OpenSSL (the .dll libraries for crypto and ssl are already included in the repository).

C++17 standard libraries or newer.
