#include "protocol.h"

Protocol::Protocol(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, Base* theBase, unsigned short debug) : serverName(serverAddress), debugLevel(debug), serverConf(confVars), botBase(theBase) {
	connection = botBase->assignSocket(confVars["sockettype"]);
}

Protocol::~Protocol() {}

void Protocol::connectServer() {}

bool Protocol::stillConnected() {
	if (connection == NULL)
		return false;
	return connection->isConnected();
}

bool Protocol::shouldReset() { return keepServer; }

bool Protocol::isClient() { return false; } // most protocol modules won't be clients

std::tr1::unordered_map<std::string, std::string> Protocol::info() { return serverConf; }

std::list<std::pair<std::string, char> > Protocol::prefixes() { return std::list<std::pair<std::string, char> > (); }

std::set<char> Protocol::channelTypes() { return std::set<char> (); }

std::vector<std::vector<std::string> > Protocol::channelModes() { return std::vector<std::vector<std::string> > (); }

std::list<std::string> Protocol::channels() { return std::list<std::string> (); }

std::string Protocol::channelTopic(std::string channel) { return ""; }

std::set<std::string> Protocol::channelUsers(std::string channel) { return std::set<std::string> (); }

std::string Protocol::userIdent(std::string user) { return ""; }

std::string Protocol::userHost(std::string user) { return ""; }

std::pair<std::string, char> Protocol::userStatus(std::string channel, std::string user) { return std::pair<char, char> ('0', ' '); }

char Protocol::compareStatus(std::set<std::string> statuses) { return std::set<std::string>(); }

void Protocol::sendMsg(std::string target, std::string message) {}
void Protocol::sendNotice(std::string target, std::string message) {}
void Protocol::setMode(std::string target, std::string mode) {}
void Protocol::removeMode(std::string target, std::string mode) {}
void Protocol::joinChannel(std::string channel, std::string key) {}
void Protocol::partChannel(std::string channel, std::string reason) {}
void Protocol::quitServer(std::string reason) {}
void Protocol::kickUser(std::string channel, std::string user, std::string reason) {}

void Protocol::sendNumeric(std::string numeric, std::string target, std::vector<std::string> numericData) {}
void Protocol::killUser(std::string user, std::string reason) {}
void Protocol::setXLine(char lineType, std::string hostmask, std::string time, std::string reason) {}
void Protocol::removeXLine(char lineType, std::string hostmask) {}

void Protocol::sendOther(std::string rawLine) {}

void Protocol::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {}
void Protocol::removeClient(std::string client) {}
std::list<std::string> Protocol::clients() { return std::list<std::string> (); }
std::tr1::unordered_map<std::string, std::string> clientInfo(std::string client) { return std::tr1::unordered_map<std::string, std::string> (); }
std::list<std::string> Protocol::userModes(std::string client) { return std::list<std::string> (); }