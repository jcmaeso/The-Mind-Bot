#include "chatController.h"
#include <iostream>

namespace ChatController {
	std::map<std::int64_t, std::shared_ptr<registeredChat_t>> chatList;

	void addChatToList(std::int64_t chatId, std::shared_ptr<registeredChat_t> chatPtr) {
		chatList[chatId] = chatPtr;
	}
	std::size_t getCount(std::int64_t chatId)
	{
		return chatList.count(chatId);
	}
	std::set<TgBot::User::Ptr> getChatUsers(std::int64_t chatId) {
		return chatList[chatId]->users;
	}
	registeredChat_ptr getActiveChat(std::int64_t chatId)
	{
		auto chat = chatList.find(chatId);
		if (chat != chatList.end()) {
			return chat->second;
		}
		return nullptr;
	}
	bool chatIsRegistered(std::int64_t chatId)
	{
		if (chatList.find(chatId) != chatList.end())
			return true;
		return false;
	}
}