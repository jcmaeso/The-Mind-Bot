#include "game.h"
#include <cstdio>
#include <vector>
#include <set>
#include <random>

#define MAX_NUMBER 100

namespace GameController {

	void testFcn() {
		printf("test");
	}

	std::map<std::int64_t, game_ptr> gameList;

	void newGame(std::shared_ptr<ChatController::registeredChat_t> activeChat) {
		auto game = std::make_shared<game_t>();
		game->activeChat = activeChat;
		game->state = 0;
		game->level = 0;
		gameList[activeChat->chat->id] = game;
	}

	bool addUserToGame(game_ptr game, TgBot::User::Ptr user) {
		if (game->state == 0) {
			game->playingUsers.insert(user);
			return true;
		}
		return false;
	}

	void endGame(game_ptr game) {
		gameList.erase(game->activeChat->chat->id);
	}

	game_ptr getGameFromChat(std::shared_ptr<ChatController::registeredChat_t> activeChat){
		if (gameList.count(activeChat->chat->id)) {
			return gameList[activeChat->chat->id];
		}
		return nullptr;
		
	}

	bool chatHasGame(std::shared_ptr<ChatController::registeredChat_t> activeChat) {
		if (gameList.count(activeChat->chat->id)) {
			return true;
		}
		return false;
	}

	bool gameIsReady(game_ptr game) {
		if (game->state == 0 && game->playingUsers.size() > 1) {
			return true;
		}
		return false;
	}

	std::shared_ptr<std::set<int>> randomNumberGenerator(int n){
		static std::mt19937 rng;
		rng.seed(std::random_device()());
		static std::uniform_int_distribution<std::mt19937::result_type> dist(1, MAX_NUMBER);
		auto randomNumbers_ptr = std::make_shared<std::set<int>>();
		int i = 0;
		while (true) {
			if ((*randomNumbers_ptr).insert(static_cast<int>(dist(rng))).second) {
				if (++i >= n)
					return randomNumbers_ptr;
			}
		}
		return nullptr;
	}

	std::shared_ptr<std::vector<std::shared_ptr<std::set<int>>>> randomNumberGenerator(int level, int numberOfPlayers) {
		auto randomNumbers = randomNumberGenerator(numberOfPlayers*level);
		auto randomNumbersSplitted = std::make_shared<std::vector<std::shared_ptr<std::set<int>>>>();
		for (int i = 0; i < numberOfPlayers; i+=level) {
			auto itBegin = (*randomNumbers).begin();
			auto itEnd = (*randomNumbers).begin();
			std::advance(itBegin, i);
			std::advance(itEnd, i + level);
			auto randomNumbersForPlayer = std::make_shared<std::set<int>>(itBegin, itEnd);
			(*randomNumbersSplitted).push_back(randomNumbersForPlayer);
		}
		return randomNumbersSplitted;
	}

	void launchGame(game_ptr game, TgBot::Bot bot) {
		//Change state and set level
		game->state = 1;
		//Generate NumberList
		for (int i = 1; i <= MAX_NUMBER; i++) {
			game->numberList.push_back(i);
		}
		//LaunchLevel1
		launchNextLevel(game,bot);
	}

	void launchNextLevel(game_ptr game, TgBot::Bot bot)
	{
		auto level = ++game->level;
		auto numberOfPlayers = static_cast<int>(game->playingUsers.size());
		auto listOfPlayersNumbers = std::vector<std::shared_ptr<std::set<int>>>();
		auto listOfPlayerKeyboard = std::vector<std::shared_ptr<GameKeyboard>>();
		//Generate random numbers
		auto randomNumbersForPlayers = randomNumberGenerator(level, numberOfPlayers);
		//Generate all keyboards and send Messages
		for (auto randomNumbersForPlayer : (*randomNumbersForPlayers)) {
			listOfPlayerKeyboard.push_back(std::make_shared<GameKeyboard>(randomNumbersForPlayer));
		}
		//Send all messages
		for (auto player : game->playingUsers) {
			std::string msgText = "Your keyboard is ready [inline mention of a user](tg://user?id="+ std::to_string(player->id)+")";
			std::string parseMode = "[inline mention of a user](tg://user?id=" + std::to_string(player->id) + ")";
			auto keyboard = (listOfPlayerKeyboard.back())->makeKeyboard();
			listOfPlayerKeyboard.pop_back();
			bot.getApi().sendMessage(game->activeChat->chat->id, msgText, false, 0, keyboard, "Markdown");
		}
	}

	
	
	GameKeyboard::GameKeyboard(std::shared_ptr<std::set<int>> numbers) {
		//Order Received Numbers
		auto vectorNumbers = std::make_shared<std::vector<int>>((*numbers).begin(), (*numbers).end());
		std::sort((*vectorNumbers).begin(), (*vectorNumbers).end());
		//Create KeyboardButtons Object
		for (auto number : *vectorNumbers) {
			buttons.push_back(createButtonfromInt(number));
		}
	}
	TgBot::ReplyKeyboardMarkup::Ptr GameKeyboard::makeKeyboard() {
		TgBot::ReplyKeyboardMarkup::Ptr replyKeyboard(new TgBot::ReplyKeyboardMarkup);
		replyKeyboard->oneTimeKeyboard = true;
		for (auto row : buttons) {
			replyKeyboard->keyboard.push_back(row);
		}
		replyKeyboard->resizeKeyboard = false;
		replyKeyboard->selective = true;
		return replyKeyboard;
	}
	std::vector<TgBot::KeyboardButton::Ptr> GameKeyboard::createButtonfromInt(int number) {
		std::vector<TgBot::KeyboardButton::Ptr> row;
		TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
		button->text = std::to_string(number);
		button->requestContact = false;
		button->requestLocation = false;
		row.push_back(button);
		return row;
	}
}