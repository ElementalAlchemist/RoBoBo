#include "connection.h"
#include "user.cpp"

#ifndef CHANNEL_ROBOBO
#define CHANNEL_ROBOBO
Channel::Channel(Server* thisServer) {
	parentServer = thisServer;
	topic = "";
	namesSync = false;
}

void Channel::parseNames(std::vector<std::string> names) {
	if (namesSync) {
		users.clear();
		namesSync = false;
	}
	// will handle this soon
}

void Channel::numeric366() {
	namesSync = true;
}

void Channel::setTopic(std::string newTopic) {
	topic = newTopic;
}

void Channel::setMode(bool add, char mode, std::string param) {
	bool handled = false;
	std::tr1::unordered_map<char, char> prefixes = parentServer->getPrefixes();
	for (std::tr1::unordered_map<char, char>::iterator it = prefixes.begin(); it != prefixes.end(); it++) {
		if (it->first == mode) {
			//users[param].status(add, mode);
			bool exists = false;
			for (std::tr1::unordered_map<std::string, User>::iterator iter = users.begin(); iter != users.end(); iter++) {
				if (iter->first == param){
					iter->second.status(add, mode);
					exists = true;
				}
			}
			if (!exists)
				parentServer->resyncChannels();
			handled = true;
		}
	}
	if (!handled) {
		std::vector<std::vector<char> > chanModes = parentServer->getChanModes();
		for (std::vector<char>::iterator it = chanModes[0].begin(); it != chanModes[0].end(); it++) {
			if (mode == *it) { // check if it's a list mode
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
	users.insert(std::pair<std::string, User> (nick, User (this, ident, host, gecos)));
}

void Channel::leaveChannel(std::string nick) {
	users.erase(nick);
}
#endif