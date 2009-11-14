#include "connection.h"

#ifndef USER_ROBOBO
#define USER_ROBOBO
User::User(Channel* thisChannel, std::string theIdent, std::string theHost, std::string theGECOS) {
	parentChannel = thisChannel;
	ident = theIdent;
	host = theHost;
	gecos = theGECOS;
	for (std::tr1::unordered_map<char, char>::iterator it = parentChannel->parentServer->prefix.begin(); it != parentChannel->parentServer->prefix.end(); it++)
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