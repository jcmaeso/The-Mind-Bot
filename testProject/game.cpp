#include "game.h"
#include <cstdio>
#include <vector>
#include <string>
#include <set>
#include <random>
#include <stdexcept>
#include <algorithm>

#define MAX_NUMBER 100


namespace GameController {

	typedef std::shared_ptr<game_t> game_ptr;

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
	//Chapuza
	TgBot::User::Ptr getPlayingUserById(game_ptr game,TgBot::User::Ptr rcvUser) {
		for (auto playerNumbers : game->playersNumbers) {
			if (playerNumbers.first->id == rcvUser->id) {
				return playerNumbers.first;
			}
		}
		return nullptr;
	}

	bool userIsPlaying(game_ptr game, TgBot::User::Ptr user)
	{
		for (auto playingUser : game->playingUsers) {
			if (playingUser->id == user->id)
				return true;
		}
		return false;
	}
	
	bool numberIsFromGame(game_ptr game, TgBot::Message::Ptr message, int number) {
		if (number > MAX_NUMBER)
			return false;
		auto numbersFromPlayer = game->playersNumbers[getPlayingUserById(game,message->from)];
		if (std::find((*numbersFromPlayer).begin(), (*numbersFromPlayer).end(), number) != (*numbersFromPlayer).end())
			return true;
		return false;
	} 

	bool messageIsFromGame(game_ptr game, TgBot::Message::Ptr message, int* numberPlayed)
	{
		//Check if content is number
		try {
			(*numberPlayed) = std::stoi(message->text);
		}
		catch (const std::invalid_argument e) {
			printf("Message is not from game (not a number)");
			return false;
		}
		catch (const std::out_of_range e) {
			printf("Message is not from game (number out of range)");
			return false;
		}
		//Check if number is from game and player
		return numberIsFromGame(game, message, (*numberPlayed));
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
		//Generate all keyboards and send Messages generate user numbers
		for (auto randomNumbersForPlayer : (*randomNumbersForPlayers)) {
			listOfPlayerKeyboard.push_back(std::make_shared<GameKeyboard>(randomNumbersForPlayer));
		}
		//Send all messages
		//TODO: SendKeyboardToPlayerFunction
		for (auto player : game->playingUsers) {
			//Add Numbers to the map
			game->playersNumbers[player] = (listOfPlayerKeyboard.back())->getNumbers();
			std::string msgText = "Your keyboard is ready [inline mention of a user](tg://user?id="+ std::to_string(player->id)+")";
			std::string parseMode = "[inline mention of a user](tg://user?id=" + std::to_string(player->id) + ")";
			auto keyboard = (listOfPlayerKeyboard.back())->makeKeyboard();
			listOfPlayerKeyboard.pop_back();
			bot.getApi().sendMessage(game->activeChat->chat->id, msgText, false, 0, keyboard, "Markdown");
		}
	}

	bool numberIsTheLowest(game_ptr game, int playedNumber, TgBot::User::Ptr resultUser) {
		int min = MAX_NUMBER+1;
		std::set<int>::iterator pos;
		TgBot::User::Ptr playerWithMinNumber;
		//Fuga De memoria
		game->playersNumbers.erase(0);
		for (auto elm : game->playersNumbers) {
			if ((*elm.second).empty()) {
				continue;
			}
			auto numbers = elm.second;
			auto min_elm = std::min_element((*numbers).begin(), (*numbers).end());
			if ((*min_elm) < min) {
				min = (*min_elm);
				pos = min_elm;
				playerWithMinNumber = elm.first;
			}
		}
		if (playedNumber == min) {
			(*game->playersNumbers[playerWithMinNumber]).erase(pos);
			resultUser = playerWithMinNumber;
			return true;
		}
		resultUser = nullptr;
		return false;
	}

	bool isEndLevel(game_ptr game) {
		auto numberOfNumbers = game->playingUsers.size()*game->level;
		if (game->playedNumbers.size() == numberOfNumbers)
			return true;
		return false;
	}

	void endGame(game_ptr game, TgBot::Bot bot) {
		bot.getApi().sendMessage(game->activeChat->chat->id, "Game has ended");
		//delete game from the list
		gameList.erase(game->activeChat->chat->id);
	}

	void processNumber(game_ptr game, TgBot::User::Ptr user, int playedNumber,TgBot::Bot bot)
	{
		//Check if its the lowest Posible
		std::shared_ptr<GameKeyboard> keyboard;
		//Check if its last number
		TgBot::User::Ptr player;
		if (numberIsTheLowest(game, playedNumber, player)) {
			//Check End Level
			game->playedNumbers.push_back(playedNumber);
			bot.getApi().sendMessage(game->activeChat->chat->id, "Number played is correct");
			if (isEndLevel(game)) {
				launchNextLevel(game, bot);
				return;
			}
			//GenerateNewKeyboard
			std::string msgText = "Your new keyboard is ready [inline mention of a user](tg://user?id=" + std::to_string(user->id) + ")";
			keyboard = std::make_shared<GameKeyboard>(game->playersNumbers[player]);
			bot.getApi().sendMessage(game->activeChat->chat->id, msgText, false, 0, keyboard->makeKeyboard(), "Markdown");
			return;
		}	
		//Lose Game
		endGame(game,bot);
	}

	GameKeyboard::GameKeyboard(std::shared_ptr<std::set<int>> numbers) {
		//Check if its empty to generate default keyboard
		if (numbers->empty()) {
			this->typeOfKeyboard = 0;
			this->numbers = nullptr;
			buttons.push_back(createEmptyButton());
			return;
		}
		this->typeOfKeyboard = 1;
		//Order Received Numbers
		this->numbers = numbers;
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
		replyKeyboard->resizeKeyboard = false;
		replyKeyboard->selective = true;
		for (auto row : buttons) {
			replyKeyboard->keyboard.push_back(row);
		}
		return replyKeyboard;
	}
	std::shared_ptr<std::set<int>> GameKeyboard::getNumbers()
	{
		return numbers;
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
	std::vector<TgBot::KeyboardButton::Ptr> GameKeyboard::createEmptyButton()
	{
		std::vector<TgBot::KeyboardButton::Ptr> row;
		TgBot::KeyboardButton::Ptr button(new TgBot::KeyboardButton);
		button->text = "Wait to the end of the level";
		button->requestContact = false;
		button->requestLocation = false;
		row.push_back(button);
		return row;
	}
}