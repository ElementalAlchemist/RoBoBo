#include "protocol.h"

Protocol::Protocol(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug) : serverName(serverAddress), serverConf(confVars), botBase(theBase), debugLevel(debug) {
	connection = theBase->assignSocket(confVars["sockettype"]);
}

Protocol::~Protocol() {}

void Protocol::connectServer() {}

bool Protocol::stillConnected() {
	return false;
}

bool Protocol::shouldReset() {
	return false;
}

bool Protocol::isClient() {
	return false; // most protocol modules won't be clients
}

std::tr1::unordered_map<std::string, std::string> Protocol::info() {
	return std::tr1::unordered_map<std::string, std::string> ();
}

std::list<std::pair<char, char> > Protocol::prefixes() { // for now; to be status class
	return std::list<std::pair<char, char> > ();
}

std::vector<std::vector<char> > Protocol::channelModes() {
	return std::vector<std::vector<char> > ();
}

std::vector<char> Protocol::channelTypes() {
	return std::vector<char> ();
}

std::list<std::string> Protocol::channels() {
	return std::list<std::string> ();
}

std::string Protocol::channelTopic(std::string channel) {
	return "";
}

std::list<std::string> Protocol::channelUsers(std::string channel) {
	return std::list<std::string> ();
}

std::string Protocol::userIdent(std::string user) {
	return "";
}

std::string Protocol::userHost(std::string user) {
	return "";
}

std::pair<char, char> Protocol::userStatus(std::string channel, std::string user) {
	return std::pair<char, char> ('0', ' ');
}

void Protocol::sendMsg(std::string target, std::string message) {}
void Protocol::sendNotice(std::string target, std::string message) {}
void Protocol::setMode(std::string target, std::string mode) {}
void Protocol::removeMode(std::string target, std::string mode) {}
void Protocol::joinChannel(std::string channel, std::string key = "") {}
void Protocol::partChannel(std::string channel, std::string reason = "") {}
void Protocol::quitServer(std::string reason) {}
void Protocol::kickUser(std::string channel, std::string user, std::string reason) {}

void Protocol::sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData) {}
void Protocol::killUser(std::string user, std::string reason) {}
void Protocol::setXLine(char lineType, std::string hostmask, std::string time, std::string reason) {}
void Protocol::removeXLine(char lineType, std::string hostmask) {}

void Protocol::sendOther(std::string rawLine) {}