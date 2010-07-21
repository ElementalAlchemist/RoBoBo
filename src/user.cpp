#include "connection.h"

#ifndef USER_CPP
#define USER_CPP
User::User(Channel* thisChannel) : parentChannel(thisChannel), ident(""), host("") {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->getPrefixes();
	for (unsigned int i = 0; i < prefixes.size(); i++)
		hasStatus.push_back(std::pair<char, bool> (prefixes[i].first, false));
}

User::User(std::string theIdent, std::string theHost, Channel* thisChannel) : parentChannel(thisChannel), ident(theIdent), host(theHost) {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->getPrefixes();
	for (unsigned int i = 0; i < prefixes.size(); i++)
		hasStatus.push_back(std::pair<char, bool> (prefixes[i].first, false));
}

void User::ident(std::string newIdent) {
	if (ident == "")
		ident = newIdent;
}

std::string User::ident() {
	return ident;
}

void User::host(std::string newHost) {
	if (host == "")
		host = newHost;
}

std::string User::host() {
	return host;
}

void User::status(bool add, char status) {
	hasStatus[status] = add;
}

char User::status() {
	for (std::map<char, bool>::iterator statusIter = hasStatus.begin(); statusIter != hasStatus.end(); ++statusIter) {
		if (statusIter->second)
			return statusIter->first;
	}
	return '0';
}
#endif