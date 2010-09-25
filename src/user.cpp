#include "connection.h"

#ifndef USER_CPP
#define USER_CPP
User::User(Channel* thisChannel) : parentChannel(thisChannel), theIdent(""), theHost("") {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->prefixes();
	for (unsigned int i = 0; i < prefixes.size(); i++)
		hasStatus.push_back(std::pair<char, bool> (prefixes[i].first, false));
}

User::User(std::string theTheIdent, std::string theTheHost, Channel* thisChannel) : parentChannel(thisChannel), theIdent(theTheIdent), theHost(theTheHost) {
	std::vector<std::pair<char, char> > prefixes = parentChannel->parentServer->prefixes();
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
	for (unsigned int i = 0; i < hasStatus.size(); i++) {
		if (hasStatus[i].first == status) {
			hasStatus[i].second = add;
			break;
		}
	}
}

char User::status() {
	for (unsigned int i = 0; i < hasStatus.size(); i++) {
		if (hasStatus[i].second)
			return hasStatus[i].first;
	}
	return '0';
}
#endif