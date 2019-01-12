#pragma once
#include <tgbot/types/Message.h>
#include <set>
#include <string>

#include "game.h"

namespace MessageController {
	typedef std::shared_ptr<std::set<std::string>> commandList_ptr;

	class MessageFilter{
	public:
		MessageFilter(commandList_ptr commands);
		bool messageIsCommand(TgBot::Message::Ptr message);
		bool messageIsFromGame(TgBot::Message::Ptr message, GameController::game_ptr game, int* number);
	private:
		commandList_ptr commands;
	};
	

}