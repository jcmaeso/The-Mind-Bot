#include "chatController.h"
#include "messageController.h"
#include "game.h"
#include <stdio.h>
#include <set>
#include "main.h"

#define START_COMAND "start"
#define INITIALIZE_COMAND "initialize"
#define STARTGAME_COMAND "startgame"
#define JOIN_COMMAND "join"
#define READY_COMMAND "ready"

std::set<std::string> commandList({ 
	START_COMAND,
	INITIALIZE_COMAND,
	STARTGAME_COMAND,
	JOIN_COMMAND,
	READY_COMMAND 
});

int main() {
	auto commandList_ptr = std::make_shared <std::set<std::string>>(commandList);
	//Define Filter Class with the list of commands
	auto messageFilter_ptr = std::make_shared<MessageController::MessageFilter>(commandList_ptr);
	TgBot::Bot bot("790754969:AAHIKutqHMILadsGqSLJKajpBJKwirekZB0");
	bot.getEvents().onCommand(START_COMAND, [&bot](TgBot::Message::Ptr message) {
		bot.getApi().sendMessage(message->chat->id, "Hi!");
	});
	bot.getEvents().onCommand(INITIALIZE_COMAND, [&bot](TgBot::Message::Ptr message) {
		if (!ChatController::getCount(message->chat->id)) {
			auto newChat = std::make_shared<ChatController::registeredChat_t>();
			newChat->chat = message->chat;
			newChat->users.insert(message->from);
			ChatController::addChatToList(message->chat->id, newChat);
			bot.getApi().sendMessage(message->chat->id, "A chat has been stored!");
		}
		else {
			bot.getApi().sendMessage(message->chat->id, "This chat has already been stored, don't use this cmd again");
		}

	});
	bot.getEvents().onCommand(STARTGAME_COMAND, [&bot](TgBot::Message::Ptr message) {
		if (message->chat->id >= 0) {
			bot.getApi().sendMessage(message->chat->id, "This is not a group chat");
			return;
		}
		auto activeChat = ChatController::getActiveChat(message->chat->id);
		if (activeChat != nullptr) {
			GameController:: newGame(activeChat);
			bot.getApi().sendMessage(message->chat->id, "Game started!!");
			printf("Game created in %d", message->chat->id);
			return;
		}
		bot.getApi().sendMessage(message->chat->id, "Chat not registered please type \\initialize command");
		printf("Game not created in %d (not initialized)", message->chat->id);
	});
	bot.getEvents().onCommand(JOIN_COMMAND, [&bot](TgBot::Message::Ptr message) {
		auto activeChat = ChatController::getActiveChat(message->chat->id);
		if (activeChat == nullptr) {
			bot.getApi().sendMessage(message->chat->id, "Chat not registered please type \\initialize command");
			printf("Game not created in %d (not initialized)", message->chat->id);
			return;
		}
		auto game = GameController::getGameFromChat(activeChat);
		if (game != nullptr) {
				GameController::addUserToGame(game, message->from);
				bot.getApi().sendMessage(message->chat->id, "Joined to the game");
				return;
		}
		bot.getApi().sendMessage(message->chat->id, "Start a game to join");
	});
	bot.getEvents().onCommand(READY_COMMAND, [&bot](TgBot::Message::Ptr message) {
		auto activeChat = ChatController::getActiveChat(message->chat->id);
		auto game = GameController::getGameFromChat(activeChat);
		if (game != nullptr) {
			if (GameController::gameIsReady(game)) {
				GameController::launchGame(game,bot);
			}
		}
	});
	bot.getEvents().onAnyMessage([&bot,messageFilter_ptr](TgBot::Message::Ptr message) {
		printf("User wrote %s\n", message->text.c_str());
		if (messageFilter_ptr->messageIsCommand(message))
			return;
		//if (ChatController::getCount(message->chat->id) && !message->from->isBot){
		//	auto chatUsers = ChatController::getChatUsers(message->chat->id);
		//	auto user = chatUsers.find(message->from);
		//	if (user != chatUsers.end()) {
		//		chatUsers.insert(message->from);
		//	}
		//	printf("User %s with %d added to chat %d", message->from->username, message->from->id, message->chat->id);
		//	//Add recent users to it.
		//}
		//Check if chat is registered
		auto avctiveChat = ChatController::getActiveChat(message->chat->id);
		if (avctiveChat == nullptr)
			return;
		//Check if chat has game
		auto game = GameController::getGameFromChat(avctiveChat);
		if (game == nullptr)
			return;
		//Check if it's message from a game
		int playedNumber;
		if (!messageFilter_ptr->messageIsFromGame(message, game, &playedNumber))
			return;
		//Process Number for Game
		GameController::processNumber(game, message->from, playedNumber, bot);
		// Echo for debug
		bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
	});
	try {
		printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
		TgBot::TgLongPoll longPoll(bot);
		while (true) {
			printf("Long poll started\n");
			longPoll.start();
		}
	}
	catch (TgBot::TgException& e) {
		printf("error: %s\n", e.what());
	}
	return 0;
}