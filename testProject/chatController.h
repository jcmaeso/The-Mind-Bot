#pragma once
#include <tgbot/tgbot.h>
#include <set>



namespace ChatController {
	typedef struct registeredChat {
		TgBot::Chat::Ptr chat;
		std::set<TgBot::User::Ptr> users;
	}registeredChat_t;

	typedef std::shared_ptr<registeredChat_t> registeredChat_ptr;

	void addChatToList(std::int64_t chatId, std::shared_ptr<registeredChat_t> chatPtr);
	std::size_t getCount(std::int64_t chatId);
	std::set<TgBot::User::Ptr> getChatUsers(std::int64_t chatId);
	registeredChat_ptr getActiveChat(std::int64_t chatId);
	bool chatIsRegistered(std::int64_t chatId);
}
