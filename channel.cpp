#include "channel.h"
#include "server.cpp"
#include "user.cpp"

#ifndef CHANNEL_ROBOBO
#define CHANNEL_ROBOBO
class Channel {
	public:
		Channel(Server theServer);
		void setTopic(std::string newTopic);
		void names(std::vector<std::string> namesLine);
		void joinUser(std::string nick);
		void statusChange(bool addStatus, std::string nick, char mode);
		void chanMode(bool addMode, std::string mode);
	private:
		std::string topic;
		std::list<User> channelUsers;
		std::list<User>::iterator userIterator;
		Server serverSettings;
		int prefixSymbolNum(char prefix);
};

Channel::Channel(Server theServer) {
	topic = "";
	serverSettings = theServer;
}

void Channel::setTopic(std::string newTopic) {
	topic = newTopic;
}

void Channel::names(std::vector<std::string> namesLine) {
	std::vector<char> prefixes = serverSettings.prefixModes(true);
	std::string currNick = "";
	std::vector<bool> levels;
	for (unsigned int i = 0; i < prefixes.size(); i++)
		levels.push_back(false);
	for (unsigned int i = 6; i < namesLine.size(); i++) {
		currNick = namesLine[i];
		while (prefixSymbolNum(currNick[0]) != -1) {
			levels[prefixSymbolNum(currNick[0])] = true;
			currNick = currNick.substr(1);
		}
		User newUser (currNick, levels);
		channelUsers.push_back(newUser);
	}
}

void Channel::joinUser(std::string nick) {
	User newUser (nick);
	channelUsers.push_back(newUser);
}

void Channel::statusChange(bool addStatus, std::string nick, char mode) {
	int statusIndex;
	std::vector<char> prefixes = serverSettings.prefixModes(false);
	for (unsigned int i = 0; i < prefixes.size(); i++) {
		if (prefixes[i] == mode) {
			statusIndex = (int)i;
			break;
		}
	}
	for (userIterator = channelUsers.begin(); userIterator != channelUsers.end(); userIterator++) {
		if (userIterator.getNick() == nick) {
			if (addStatus)
				userIterator.addStatus(statusIndex);
			else
				userIterator.removeStatus(statusIndex);
			break;
		}
	}
}

int Channel::prefixSymbolNum(char prefix) {
	std::vector<char> prefixes = serverSettings.prefixModes(true);
	for (unsigned int i = 0; i < prefixes.size(); i++)
		if (prefixes[i] == prefix)
			return (int) i;
	return -1;
}
#endif