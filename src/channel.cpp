#include "connection.h"

Channel::Channel(Server* thisServer) : parentServer(thisServer), topic(""), namesSync(false) {}

void Channel::parseNames(std::vector<std::string> names) {
	if (namesSync) {
		users.clear();
		namesSync = false;
	}
	std::tr1::unordered_map<char, char> prefixes = parentServer->getPrefixes();
	std::vector<char> modes;
	for (unsigned int i = 0; i < names.size(); i++) {
		for (std::tr1::unordered_map<char, char>::iterator prefixIter = prefixes.begin(); prefixIter != prefixes.end(); prefixIter++) {
			if (prefixIter->second == names[i][0]) {
				modes.push_back(prefixIter->first);
				names[i] = names[i].substr(1);
			}
		}
		users.insert(std::pair<std::string, User*> (names[i], new User (this)));
		std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(names[i]);
		if (userIter == users.end())
			continue;
		for (unsigned int j = 0; j < modes.size(); j++)
			userIter->second->status(true, modes[j]);
		modes.clear();
	}
}

void Channel::numeric366() {
	namesSync = true;
}

void Channel::topic(std::string newTopic) {
	topic = newTopic;
}

void Channel::mode(bool add, char mode, std::string param) {
	std::tr1::unordered_map<char, char> prefixes = parentServer->getPrefixes();
	std::tr1::unordered_map<char, char>::iterator it = prefixes.find(mode);
	if (it == prefixes.end())
		return;
	std::tr1::unordered_map<std::string, User*>::iterator iter = users.find(param);
	if (iter == users.end()) {
		parentServer->resyncChannels();
		return;
	}
	iter->second->status(add, mode);
}

void Channel::joinChannel(std::string hostmask) {
	std::string nick = hostmask.substr(0, hostmask.find_first_of('!'));
	std::string ident = hostmask.substr(hostmask.find_first_of('!') + 1);
	ident = ident.substr(0, ident.find_first_of('@'));
	std::string host = hostmask.substr(hostmask.find_first_of('@') + 1);
	users.insert(std::pair<std::string, User*> (nick, new User (ident, host, this)));
}

void Channel::leaveChannel(std::string nick) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(nick);
	if (userIter == users.end())
		return;
	delete userIter->second;
	users.erase(nick);
}

void Channel::nick(std::string oldNick, std::string newNick) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(nick);
	if (userIter == users.end())
		return;
	User* userData = userIter->second;
	users.erase(userIter);
	users.insert(std::pair<std::string, User*> (newNick, userData));
}

std::list<std::string> Channel::users() {
	std::list<std::string> channelUsers;
	for (std::tr1::unordered_map<std::string, User*>::iterator userIter = users.begin(); userIter != users.end(); ++userIter)
		channelUsers.insert(channelUsers.end(), userIter->first);
	return channelUsers;
}

void Channel::ident(std::string user, std::string ident) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return;
	userIter->second->ident(ident);
}

std::string Channel::ident(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->ident();
}

void Channel::host(std::string user, std::string host) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return;
	userIter->second->host(host);
}

std::string Channel::host(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return "";
	return userIter->second->host();
}

char Channel::status(std::string user) {
	std::tr1::unordered_map<std::string, User*>::iterator userIter = users.find(user);
	if (userIter == users.end())
		return '0';
	return userIter->second->getStatus();
}

std::string Channel::topic() {
	return topic;
}