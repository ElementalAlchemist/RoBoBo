#include "connection.h"

#ifndef USER_CPP
#define USER_CPP
User::User(Channel* thisChannel) {
	parentChannel = thisChannel;
	std::tr1::unordered_map<char, char> prefixes = parentChannel->parentServer->getPrefixes();
	for (std::tr1::unordered_map<char, char>::iterator it = prefixes.begin(); it != prefixes.end(); it++)
		hasStatus.insert(std::pair<char, bool> (it->first, false));
}

void User::status(bool add, char status) {
	hasStatus[status] = add;
}
#endif