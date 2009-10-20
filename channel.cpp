#include "channel.h"

#ifndef CHANNEL_ROBOBO
#define CHANNEL_ROBOBO
class Channel {
	public:
		Channel(Server& thisServer);
		void parseNames(std::vector<std::string> names);
		void channelJoin(std::string hostmask);
		void channelLeave(std::string hostmask, std::string reason);
		void statusChange(std::string nick, char status, bool add);
		Server server;
	private:
		std::list<User> users;
		std::vector<std::string> parseHostmask(std::string hostmask);
};

Channel::Channel(Server& thisServer) {
	&server = &thisServer;
}

void Channel::parseNames(std::vector<std::string> names) {
	std::vector<char> statusSymbols;
	std::map<char, char>::iterator symbolFinder;
	for (symbolFinder = server.statusModes.begin(); symbolFinder != server.statusModes.end(); symbolFinder++)
		statusSymbols.push_back(symbolFinder->second);
	
	for (unsigned int i = 0; i < names.size(); i++) {
		bool startsWithStatus = false;
		for (unsigned int j = 0; j < statusSymbols.size(); j++) {
			if (names[i][0] == statusSymbols[j])
				startsWithStatus = true;
		}
		std::vector<char> possessedStatuses;
		while (startsWithStatus) {
			possessedStatuses.push_back(names[i][0]);
			names[i] = names[i].substr(1);
			for (unsigned int j = 0; j < statusSymbols.size(); j++) {
				if (names[i][0] == statusSymbols[j])
					startsWithStatus = true;
			}
		}
		users.insert(users.end(), User (names[i]));
		std::list<User>::iterator usersEnd = users.end() - 1;
		usersEnd->addStatuses(possessedStatuses);
	}
}

void Channel::channelJoin(std::string hostmask) {
	std::vector<std::string> nuh = parseHostmask(hostmask);
	users.insert(users.end(), User(nuh[0]));
	std::list<User>::iterator userIterator = users.end() - 1;
	userIterator->setIdent(nuh[1]);
	userIterator->setHost(nuh[2]);
}

void Channel::channelLeave(std::string hostmask, std::string reason) {
	std::list<User>::iterator userIterator;
	std::string userHostmask;
	for (userIterator = users.start(); userIterator != users.end(); userIterator++) {
		userHostmask = userIterator->getNick() + "!" + userIterator->getIdent() + "@" + userIterator->getHost();
		if (hostmask == userHostmask)
			users.erase(userIterator);
	}
}

void Channel::statusChange(std::string nick, char status, bool add) {
	std::list<User>::iterator userIterator;
	for (userIterator = users.start(); userIterator != users.end(); userIterator++) {
		if (userIterator->getNick() == nick)
			userIterator->statusChange(status, add);
	}
}

std::vector<std::string> parseHostmask(std::string hostmask) {
	std::string hostmaskPart;
	std::vector<std::string> splitMask;
	for (unsigned int i = 0; i < hostmask.size(); i++) {
		if (hostmask[i] == '!' || hostmask[i] == '@') // end of nick or ident
			splitMask.push_back(hostmaskPart);
		else
			hostmaskPart += hostmask[i];
	}
	splitMask.push_back(hostmaskPart); // push host
	return splitMask;
}
#endif