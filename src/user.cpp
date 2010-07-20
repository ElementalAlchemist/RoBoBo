#include "connection.h"

#ifndef USER_CPP
#define USER_CPP
User::User(Channel* thisChannel) : parentChannel(thisChannel), host("") {
	std::tr1::unordered_map<char, char> prefixes = parentChannel->parentServer->getPrefixes();
	for (std::tr1::unordered_map<char, char>::iterator it = prefixes.begin(); it != prefixes.end(); ++it)
		hasStatus.insert(std::pair<char, bool> (it->first, false));
}

void User::host(std::string newHost) {
	if (host == "")
		host = newHost;
}

std::string User::getHost() {
	return host;
}

void User::status(bool add, char status) {
	hasStatus[status] = add;
}

char User::getStatus() {
	for (std::map<char, bool>::iterator statusIter = hasStatus.begin(); statusIter != hasStatus.end(); ++statusIter) {
		if (statusIter->second)
			return statusIter->first;
	}
	return '0';
}
#endif