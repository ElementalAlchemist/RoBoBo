#include "connection.h"

#ifndef CHANNEL_ROBOBO
#define CHANNEL_ROBOBO
Channel::Channel(Server* thisServer) {
	parentServer = thisServer;
	topic = "";
}

void Channel::parseNames(std::string numeric353) {
	// will handle this soon
}

void Channel::setTopic(std::string newTopic) {
	topic = newTopic;
}

void Channel::setMode(bool add, char mode, std::string param) {
	bool handled = false;
	for (std::tr1::unordered_map<char, char>::iterator it = parentServer->prefix.begin(); it != parentServer->prefix.end(); it++) {
		if (it->first == mode) {
			users[param].status(add, mode);
			handled = true;
		}
	}
	if (!handled) {
		for (std::vector<char>::iterator it = parentServer->chanModes[0].begin(); it != parentServer->chanModes[0].end(); it++) {
			if (mode == *it) {
				// add to ban list, except list, or invex list
				handled = true;
			}
		}
	}
	if (!handled) {
		if (add)
			modes.insert(std::pair<char, std::string> (mode, param));
		else
			modes.erase(mode);
	}
}

void Channel::joinChannel(std::string nick, std::string ident, std::string host, std::string gecos) {
	users.insert(std::pair<std::string, User> (nick, User (ident, host, gecos)));
}

void Channel::leaveChannel(std::string nick) {
	users.erase(nick);
}
#endif