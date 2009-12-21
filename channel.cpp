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
		}
	}
}

void Channel::joinChannel(std::string nick, std::string ident, std::string host, std::string gecos) {
	users.insert(std::pair<std::string, User> (nick, User (this, ident, host, gecos)));
}

void Channel::leaveChannel(std::string nick) {
	users.erase(nick);
}
#endif