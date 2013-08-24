#include "user.h"

User::User(std::string&& id, std::string&& nick, std::string&& ident, std::string&& gecos)
	: userID(std::forward<std::string> (id)), userNick(std::forward<std::string> (nick)),
	userIdent(std::forward<std::string> (ident)), userGecos(std::forward<std::string> (gecos)),
	userAway(false) {}

User::~User() {}

std::string User::id() const {
	return userID;
}

std::string User::nick() const {
	return userNick;
}

void User::nick(const std::string& nick) {
	userNick = nick;
}

std::string User::ident() const {
	return userIdent;
}

void User::ident(const std::string& ident) {
	userIdent = ident;
}

std::string User::host() const {
	return userHost;
}

void User::host(const std::string& host) {
	userHost = host;
}

std::string User::gecos() const {
	return userGecos;
}

void User::gecos(const std::string& gecos) {
	userGecos = gecos;
}

bool User::away() const {
	return userAway;
}

std::string User::awayReason() const {
	return userAwayReason;
}

void User::setAway(const std::string& reason) {
	userAway = true;
	userAwayReason = reason;
}

void User::setUnaway() {
	userAway = false;
	userAwayReason.clear();
}

std::set<std::string> User::channels() const {
	return userChannels;
}

bool User::inChan(const std::string& channel) const {
	return userChannels.find(channel) != userChannels.end();
}

void User::addToChan(const std::string& channel) {
	userChannels.insert(channel);
}

void User::removeFromChan(const std::string& channel) {
	userChannels.erase(channel);
}