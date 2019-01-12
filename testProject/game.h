#pragma once
#include "chatController.h"
#include <set>


namespace GameController {
	typedef struct game {
		std::shared_ptr<ChatController::registeredChat_t> activeChat;
		std::set<TgBot::User::Ptr> playingUsers;
		std::vector<int> numberList;
		std::vector<int> playedNumbers;
		int state;
		int level;
	}game_t;

	void testFcn();

	typedef std::shared_ptr<game_t> game_ptr;


	void newGame(std::shared_ptr<ChatController::registeredChat_t> activeChat);

	bool addUserToGame(game_ptr game, TgBot::User::Ptr user);

	void endGame(game_ptr game);

	game_ptr getGameFromChat(std::shared_ptr<ChatController::registeredChat_t> activeChat);

	bool chatHasGame(std::shared_ptr<ChatController::registeredChat_t> activeChat);
	bool gameIsReady(game_ptr game);
	void launchGame(game_ptr game, TgBot::Bot bot);
	void launchNextLevel(game_ptr game, TgBot::Bot bot);
	class GameKeyboard {
	public:
		GameKeyboard(std::shared_ptr<std::set<int>> numbers);
		TgBot::ReplyKeyboardMarkup::Ptr makeKeyboard();
	private:
		std::vector<std::vector<TgBot::KeyboardButton::Ptr>> buttons;
		std::vector<TgBot::KeyboardButton::Ptr> createButtonfromInt(int number);
	};

}