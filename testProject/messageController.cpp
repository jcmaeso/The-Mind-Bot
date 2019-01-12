#include "messageController.h"
#include <tgbot/tgbot.h>

MessageController::MessageFilter::MessageFilter(commandList_ptr commands)
{
	this->commands = commands;
}

bool MessageController::MessageFilter::messageIsCommand(TgBot::Message::Ptr message)
{
	for (auto command : (*commands)) {
		if (StringTools::startsWith(message->text, "/" + command))
			return true;
	}
	return false;
}

bool MessageController::MessageFilter::messageIsFromGame(TgBot::Message::Ptr message, GameController::game_ptr game, int* number)
{
	//User is playing
	if (!GameController::userIsPlaying(game, message->from))
		return false;
	//Message is a number from the game
	return GameController::messageIsFromGame(game, message, number);
}
