#include "connection.h"

#ifndef USER_CPP
#define USER_CPP
User::User(Channel* thisChannel) : parentChannel(thisChannel), theIdent(""), theHost("") {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->getPrefixes();
	for (unsigned int i = 0; i < prefixes.size(); i++)
		hasStatus.push_back(std::pair<char, bool> (prefixes[i].first, false));
}

User::User(std::string theTheIdent, std::string theTheHost, Channel* thisChannel) : parentChannel(thisChannel), theIdent(theTheIdent), theHost(theTheHost) {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->getPrefixes();
	for (unsigned int i = 0; i < prefixes.size(); i++)
		hasStatus.push_back(std::pair<char, bool> (prefixes[i].first, false));
}

void User::ident(std::string newIdent) {
	if (theIdent == "")
		theIdent = newIdent;
}

std::string User::ident() {
	return theIdent;
}

void User::host(std::string newHost) {
	if (theHost == "")
		theHost = newHost;
}

std::string User::host() {
	return theHost;
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