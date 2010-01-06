#include "connection.h"
#include "user.cpp"

#ifndef CHANNEL_CPP
#define CHANNEL_CPP
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
		for (unsigned int j = 0; j < modes.size(); j++) {
			for (std::tr1::unordered_map<std::string, User*>::iterator userIter = users.begin(); userIter != users.end(); userIter++) {
				if (names[i] == userIter->first)
					userIter->second->status(true, modes[j]);
			}
		}
		modes.clear();
	}
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
			bool exists = false;
			for (std::tr1::unordered_map<std::string, User*>::iterator iter = users.begin(); iter != users.end(); iter++) {
				if (iter->first == param) {
					iter->second->status(add, mode);
					exists = true;
				}
			}
			if (!exists)
				parentServer->resyncChannels();
		}
	}
}

void Channel::joinChannel(std::string nick) {
	users.insert(std::pair<std::string, User*> (nick, new User (this)));
}

void Channel::leaveChannel(std::string nick) {
	users.erase(nick);
}
#endif