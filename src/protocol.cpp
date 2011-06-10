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

std::string Protocol::compareStatus(std::set<std::string> statuses) { return std::set<std::string>(); }

void Protocol::sendMsg(std::string target, std::string message) {}
void Protocol::sendNotice(std::string target, std::string message) {}
void Protocol::setMode(std::string target, std::string mode) {}
void Protocol::removeMode(std::string target, std::string mode) {}
void Protocol::joinChannel(std::string channel, std::string key) {}
void Protocol::partChannel(std::string channel, std::string reason) {}
void Protocol::quitServer(std::string reason) {}
void Protocol::kickUser(std::string channel, std::string user, std::string reason) {}
void Protocol::changeNick(std::string client, std::string newNick) {}
void Protocol::oper(std::string client, std::string username, std::string password) {}

void Protocol::killUser(std::string user, std::string reason) {}
void Protocol::setXLine(char lineType, std::string hostmask, time_t duration, std::string reason) {}
void Protocol::removeXLine(char lineType, std::string hostmask) {}

std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > Protocol::listXLines() {
	return std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > ();
}

void Protocol::sendSNotice(char snomask, std::string text) {}

void Protocol::sendOther(std::string rawLine) {}

std::string Protocol::addClient(std::string nick, std::string ident, std::string host, std::string gecos) { return ""; }
void Protocol::removeClient(std::string client, std::string reason) {}
std::list<std::string> Protocol::clients() { return std::list<std::string> (); }
std::tr1::unordered_map<std::string, std::string> Protocol::clientInfo(std::string client) { return std::tr1::unordered_map<std::string, std::string> (); }
std::list<std::string> Protocol::userModes(std::string client) { return std::list<std::string> (); }

std::vector<std::string> Protocol::parseLine(std::string rawLine) {
	std::vector<std::string> parsedLine;
	std::string linePart = "";
	for (unsigned int i = 0; i < rawLine.size(); i++) {
		if (i != 0 && rawLine[i] == ':' && rawLine[i-1] == ' ') {
			while (i < rawLine.size())
				linePart += rawLine[i];
			parsedLine.push_back(linePart);
			return parsedLine;
		}
		if (rawLine[i] == ' ') {
			parsedLine.push_back(linePart);
			linePart = "";
			continue;
		}
		linePart += rawLine[i];
	}
	if (linePart != "")
		parsedLine.push_back(linePart);
	return parsedLine;
}

bool Protocol::callChanMsgHook(std::string client, std::string channel, char target, std::string nick, std::string message) {
	return botBase->callChanMsgHook(serverName, client, channel, target, nick, message);
}

bool Protocol::callUserMsgHook(std::string client, std::string nick, std::string message) {
	return botBase->callUserMsgHook(serverName, client, nick, message);
}

bool Protocol::callChanNoticeHook(std::string client, std::string channel, char target, std::string nick, std::string message) {
	return botBase->callChanNoticeHook(serverName, client, channel, target, nick, message);
}

bool Protocol::callUserNoticeHook(std::string client, std::string nick, std::string message) {
	return botBase->callUserNoticeHook(serverName, client, nick, message);
}

bool Protocol::callChannelCTCPHook(std::string client, std::string channel, char target, std::string nick, std::string message) {
	return botBase->callChannelCTCPHook(serverName, client, channel, target, nick, message);
}

bool Protocol::callUserCTCPHook(std::string client, std::string nick, std::string message) {
	return botBase->callUserCTCPHook(serverName, client, nick, message);
}

bool Protocol::callChannelCTCPReplyHook(std::string client, std::string channel, char target, std::string nick, std::string message) {
	return botBase->callChannelCTCPReplyHook(serverName, client, channel, target, nick, message);
}

bool Protocol::callUserCTCPReplyHook(std::string client, std::string nick, std::string message) {
	return botBase->callUserCTCPReplyHook(serverName, client, nick, message);
}

void Protocol::callChannelJoinPreHook(std::string client, std::string channel, std::string hostmask) {
	botBase->callChannelJoinPreHook(serverName, client, channel, hostmask);
}

void Protocol::callChannelJoinPostHook(std::string client, std::string channel, std::string hostmask) {
	botBase->callChannelJoinPostHook(serverName, client, channel, hostmask);
}

void Protocol::callChannelPartPreHook(std::string client, std::string channel, std::string hostmask, std::string reason) {
	botBase->callChannelPartPreHook(serverName, client, channel, hostmask, reason);
}

void Protocol::callChannelPartPostHook(std::string client, std::string channel, std::string hostmask, std::string reason) {
	botBase->callChannelPartPostHook(serverName, client, channel, hostmask, reason);
}

void Protocol::callUserQuitPreHook(std::string client, std::string hostmask, std::string reason) {
	botBase->callUserQuitPreHook(serverName, client, hostmask, reason);
}

void Protocol::callUserQuitPostHook(std::string client, std::string hostmask, std::string reason) {
	botBase->callUserQuitPostHook(serverName, client, hostmask, reason);
}

void Protocol::callNickChangePreHook(std::string client, std::string oldNick, std::string newNick) {
	botBase->callNickChangePreHook(serverName, client, oldNick, newNick);
}

void Protocol::callNickChangePostHook(std::string client, std::string oldNick, std::string newNick) {
	botBase->callNickChangePostHook(serverName, client, oldNick, newNick);
}

void Protocol::callChannelKickPreHook(std::string client, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	botBase->callChannelKickPreHook(serverName, client, channel, kicker, kickee, reason);
}

void Protocol::callChannelKickPostHook(std::string client, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	botBase->callChannelKickPostHook(serverName, client, channel, kicker, kickee, reason);
}

void Protocol::callChannelModePreHook(std::string client, std::string channel, std::string setter, std::string mode, bool add, std::string param) {
	botBase->callChannelModePreHook(serverName, client, channel, setter, mode, add, param);
}

void Protocol::callChannelModePostHook(std::string client, std::string channel, std::string setter, std::string mode, bool add, std::string param) {
	botBase->callChannelModePostHook(serverName, client, channel, setter, mode, add, param);
}

void Protocol::callNumericHook(std::string client, std::string numeric, std::vector<std::string> parsedLine) {
	botBase->callNumericHook(serverName, client, numeric, parsedLine);
}

void Protocol::callOtherDataHook(std::string client, std::vector<std::string> parsedLine) {
	botBase->callOtherDataHook(serverName, client, parsedLine);
}

void Protocol::callPreConnectHook(std::string client) {
	botBase->callPreConnectHook(serverName, client);
}

void Protocol::callConnectHook(std::string client) {
	botBase->callConnectHook(serverName, client);
}

void Protocol::callQuitHook(std::string client) {
	botBase->callQuitHook(serverName, client);
}

std::string Protocol::callChannelMessageOutHook(std::string client, std::string target, char status, std::string message) {
	return botBase->callChannelMessageOutHook(serverName, client, target, status, message);
}

void Protocol::callChannelMessageSendHook(std::string client, std::string target, char status, stdd::string message) {
	botBase->callChannelMessageSendHook(serverName, client, target, status, message);
}

std::string Protocol::callUserMessageOutHook(std::string client, std::string target, std::string message) {
	return botBase->callUserMessageOutHook(serverName, client, target, message);
}

void Protocol::callUserMessageSendHook(std::string client, std::string target, std::string message) {
	botBase->callUserMessageSendHook(serverName, client, target, message);
}

std::string Protocol::callChannelNoticeOutHook(std::string client, std::string target, char status, std::string message) {
	return botBase->callChannelNoticeOutHook(serverName, client, target, status, message);
}

void Protocol::callChannelNoticeSendHook(std::string client, std::string target, char status, std::string message) {
	botBase->callChannelNoticeSendHook(serverName, client, target, status, message);
}

std::string Protocol::callUserNoticeOutHook(std::string client, std::string target, std::string message) {
	return botBase->callUserNoticeOutHook(serverName, client, target, message);
}

void Protocol::callUserNoticeSendHook(std::string client, std::string target, std::string message) {
	botBase->callUserNoticeSendHook(serverName, client, target, message);
}

std::string Protocol::callChannelCTCPOutHook(std::string client, std::string target, char status, std::string message) {
	return botBase->callChannelCTCPOutHook(serverName, client, target, status, message);
}

void Protocol::callChannelCTCPSendHook(std::string client, std::string target, char status, std::string message) {
	botBase->callChannelCTCPSendHook(serverName, client, target, status, message);
}

std::string Protocol::callUserCTCPOutHook(std::string client, std::string target, std::string message) {
	return botBase->callUserCTCPOutHook(serverName, client, target, message);
}

void Protocol::callUserCTCPSendHook(std::string client, std::string target, std::string message) {
	botBase->callUserCTCPSendHook(serverName, client, target, message);
}

std::string Protocol::callChannelCTCPReplyOutHook(std::string client, std::string target, char status, std::string message) {
	return botBase->callChannelCTCPReplyOutHook(serverName, client, target, status, message);
}

void Protocol::callChannelCTCPReplySendHook(std::string client, std::string target, char status, std::string message) {
	botBase->callChannelCTCPReplySendHook(serverName, client, target, status, message);
}

std::string Protocol::callUserCTCPReplyOutHook(std::string client, std::string target, std::string message) {
	return botBase->callUserCTCPReplyOutHook(serverName, client, target, message);
}

void Protocol::callUserCTCPReplySendHook(std::string client, std::string target, std::string message) {
	botBase->callUserCTCPReplySendHook(serverName, client, target, message);
}