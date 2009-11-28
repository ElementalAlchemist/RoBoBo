#include "connection.h"

#ifndef USER_ROBOBO
#define USER_ROBOBO
User::User(Channel* thisChannel, std::string theIdent, std::string theHost, std::string theGECOS) {
	parentChannel = thisChannel;
	ident = theIdent;
	host = theHost;
	gecos = theGECOS;
	std::tr1::unordered_map<char, char> prefixes = parentChannel->parentServer->getPrefixes();
	for (std::tr1::unordered_map<char, char>::iterator it = prefixes.begin(); it != prefixes.end(); it++)
		hasStatus.insert(std::pair<char, bool> (it->first, false));
}

std::string User::getIdent() {
	return ident;
}

std::string User::getHost() {
	return host;
}

std::string User::getName() {
	return gecos;
}

void User::status(bool add, char status) {
	hasStatus[status] = add;
}
#endif