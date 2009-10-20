#include "user.h"

#ifndef USERS_ROBOBO
#define USERS_ROBOBO
class User {
	public:
		User(Channel& thisChannel, std::string userNick);
		void addStatuses(std::vector<char> statuses);
		void statusChange(char status, bool add);
		std::string getNick();
		std::string getIdent();
		std::string getHost();
		std::string getName();
		Channel channel;
	private:
		std::string nick;
		std::string ident;
		std::string host;
		std::string gecos;
		std::tr1::unordered_map<char, bool> hasStatuses;
};

User::User(Channel& thisChannel, std::string nick) {
	nick = userNick;
	&channel = &thisChannel;
	std::tr1::unordered_map<char, char>::iterator smIterator;
	for (smIterator = channel.server.statusModes.begin(); smIterator != channel.server.statusModes.end(); smIterator++) {
		std::pair<char, bool> status (smIterator->first, false);
		hasStatuses.insert(status);
	}
}

void User::addStatuses(std::vector<char> statuses) {
	std::tr1::unordered_map<char, char>::iterator smIterator;
	for (smIterator = channel.server.statusModes.begin(); smIterator != channel.server.statusModes.end(); smIterator++) {
		for (unsigned int i = 0; i < statuses.size(); i++) {
			if (smIterator->second == statuses[i]) {
				hasStatuses[smIterator->first] = true;
				break;
			}
		}
	}
}

void User::statusChange(char status, bool add) {
	if (add)
		hasStatuses[status] = true;
	else
		hasStatuses[status] = false;
}

std::string getNick() {
	return nick;
}

std::string getIdent() {
	return ident;
}

std::string getHost() {
	return host;
}

std::string getName() {
	return gecos;
}
#endif