#include "channel.h"
#include "server.cpp"
#include "user.cpp"

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
		std::list<User> channelUsers ();
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
	unsigned int levelLength = prefixes.size();
	std::vector<bool> levels;
	for (unsigned int i = 0; i < prefixes.size(); i++)
		levels.push_back(false);
	for (unsigned int i = 6; i < namesLine.size(); i++) {
		currNick = namesLine[i];
		while (prefixSymbolNum(currNick[0]) != -1) {
			levels[prefixSymbolNum(currNick[0])] = true;
			currNick = currNick.substr(1);
		}
		User newUser = new User (currNick, levels);
		channelUsers.push_back(newUser);
	}
}

void joinUser(std::string nick) {
	User newUser = new User (nick);
	channelUsers.push_back(newUser);
}

void statusChange(bool addStatus, std::string nick, char mode) {
	unsigned int statusIndex, userIndex;
	for (unsigned int i = 0; i < serverSettings.statusModes.size(); i++) {
		if (serverSettings.statusModes[i][0] == mode) {
			statusIndex = i;
			break;
		}
	}
	for (unsigned int i = 0; i < channelUsers.size(); i++) {
		if (channelUsers[i].getNick() == nick) {
			userIndex = i;
			break;
		}
	}
	if (addStatus)
		channelUsers[userIndex].addStatus(statusIndex);
	else
		channelUsers[userIndex].removeStatus(statusIndex);
}

int Channel::prefixSymbolNum(char prefix) {
	std::vector<char> prefixes = serverSettings.prefixModes(true);
	for (unsigned int i = 0; i < prefixes.size(); i++)
		if (prefixes[i] == prefix)
			return (int) i;
	return -1;
}