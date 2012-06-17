#include "protocol.h"

Protocol::Protocol(std::string server, std::map<std::string, std::string> conf, std::string workDir, bool dumpLogs, unsigned short debug, Base* botptr) : serverName(server), workingDir(workDir), config(conf), log(dumpLogs), debugLevel(debug), bot(botptr), resetConnection(true) {}

Protocol::~Protocol() {}

void Protocol::connectServer() {}

void Protocol::disconnectServer() {}

bool Protocol::isConnected() {
	return false;
}

bool Protocol::shouldReset() {
	return resetConnection;
}

bool Protocol::isClient() {
	return false;
}

void Protocol::sendPrivMsg(std::string client, std::string target, std::string message) {}

void Protocol::sendNotice(std::string client, std::string target, std::string message) {}

void Protocol::sendCTCP(std::string client, std::string target, std::string ctcp) {}

void Protocol::sendCTCPReply(std::string client, std::string target, std::string ctcp) {}

void Protocol::setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes) {}

void Protocol::setSNOMask(std::string client, char snomask, bool add) {}

void Protocol::setChanTopic(std::string client, std::string channel, std::string topic) {}

void Protocol::joinChannel(std::string client, std::string channel, std::string key) {}

void Protocol::partChannel(std::string client, std::string channel, std::string reason) {}

void Protocol::kickUser(std::string client, std::string channel, std::string nick, std::string reason) {}

void Protocol::changeNick(std::string client, std::string newNick) {}

std::string Protocol::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	return "";
}

void Protocol::removeClient(std::string client) {}

void Protocol::oper(std::string client, std::string username, std::string password) {}

void Protocol::sendSNotice(char snomask, std::string message) {}

void Protocol::setMetadata(std::string target, std::string key, std::string value) {}

void Protocol::setXLine(std::string client, std::string linetype, std::string mask, time_t duration, std::string reason) {}

void Protocol::delXLine(std::string client, std::string linetype, std::string mask) {}

void Protocol::sendOtherData(std::string client, std::string line) {}

void Protocol::processedChanMsg(std::string client, std::string target, char status, std::string message) {}

void Protocol::processedUserMsg(std::string client, std::string target, std::string message) {}

void Protocol::processedChanNotice(std::string client, std::string target, char status, std::string message) {}

void Protocol::processedUserNotice(std::string client, std::string target, std::string message) {}

void Protocol::processedChanCTCP(std::string client, std::string target, char status, std::string ctcp) {}

void Protocol::processedUserCTCP(std::string client, std::string target, std::string ctcp) {}

void Protocol::processedChanCTCPReply(std::string client, std::string target, char status, std::string ctcp) {}

void Protocol::processedUserCTCPReply(std::string client, std::string target, std::string ctcp) {}

std::list<std::string> Protocol::listModes() {
	return std::list<std::string> ();
}

std::list<std::string> Protocol::paramModes() {
	return std::list<std::string> ();
}

std::list<std::string> Protocol::modes() {
	return std::list<std::string> ();
}

std::list<std::pair<std::string, char>> Protocol::statuses() {
	return std::list<std::pair<std::string, char>> ();
}

std::list<std::string> Protocol::channels() {
	return std::list<std::string> ();
}

std::list<std::string> Protocol::inChannels(std::string client) {
	return std::list<std::string> ();
}

std::list<std::string> Protocol::channelUsers(std::string channel) {
	return std::list<std::string> ();
}

bool Protocol::userInChannel(std::string channel, std::string user) {
	return false;
}

std::string Protocol::channelTopic(std::string channel) {
	return "";
}

std::list<std::string> Protocol::channelModes(std::string channel) {
	return std::list<std::string> ();
}

bool Protocol::channelHasMode(std::string channel, std::string mode) {
	return false;
}

std::string Protocol::modeParam(std::string channel, std::string mode) {
	return "";
}

std::list<std::string> Protocol::channelListMode(std::string channel, std::string mode) {
	return std::list<std::string> ();
}

bool Protocol::channelListHasEntry(std::string channel, std::string listMode, std::string entry) {
	return false;
}

std::pair<std::string, char> Protocol::userStatus(std::string channel, std::string user) {
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Protocol::compareStatus(std::string status0, std::string status1) {
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Protocol::compareStatus(std::string server, std::string status0, char status1) {
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Protocol::compareStatus(char status0, std::string status1) {
	return std::pair<std::string, char> ("", ' ');
}

std::pair<std::string, char> Protocol::compareStatus(char status0, char status1) {
	return std::pair<std::string, char> ("", ' ');
}

bool Protocol::userHasStatus(std::string channel, std::string user, std::string status) {
	return false;
}

bool Protocol::userHasStatus(std::string channel, std::string user, char status) {
	return false;
}

bool Protocol::userHasStatusOrGreater(std::string channel, std::string user, std::string status) {
	return false;
}

bool Protocol::userHasStatusOrGreater(std::string channel, std::string user, char status) {
	return false;
}

std::list<std::string> Protocol::clients() {
	return std::list<std::string> ();
}

std::list<std::string> Protocol::userModes(std::string client) {
	return std::list<std::string> ();
}

bool Protocol::hasUserMode(std::string client, std::string mode) {
	return false;
}

std::list<char> Protocol::snomasks(std::string client) {
	return std::list<char> ();
}

bool Protocol::hasSNOMask(std::string client, char snomask) {
	return false;
}

std::list<std::string> Protocol::userChannels(std::string nick) {
	return std::list<std::string> ();
}

Socket* Protocol::assignSocket(std::string socketType) {
	return bot->loadSocket(socketType);
}

void Protocol::killSocket(std::string socketType, Socket* sockptr) {
	bot->unloadSocket(socketType, sockptr);
}

void Protocol::callChanMsgHook(std::string client, std::string channel, char status, std::string nick, std::string message) {
	bot->modChanMsgHook(serverName, client, channel, status, nick, message);
}

void Protocol::callUserMsgHook(std::string client, std::string nick, std::string message) {
	bot->modUserMsgHook(serverName, client, nick, message);
}

void Protocol::callChanNoticeHook(std::string client, std::string channel, char status, std::string nick, std::string message) {
	bot->modChanNoticeHook(serverName, client, channel, status, nick, message);
}

void Protocol::callUserNoticeHook(std::string client, std::string nick, std::string message) {
	bot->modUserNoticeHook(serverName, client, nick, message);
}

void Protocol::callChanCTCPHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPHook(serverName, client, channel, status, nick, ctcp, "");
	else
		bot->modChanCTCPHook(serverName, client, channel, status, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callChanCTCPReplyHook(std::string client, std::string channel, char status, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPReplyHook(serverName, client, channel, status, nick, ctcp, "");
	else
		bot->modChanCTCPReplyHook(serverName, client, channel, status, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPReplyHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPReplyHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPReplyHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callChanModeHook(std::string channel, bool add, std::string mode, std::string param) {
	bot->modChanModeHook(serverName, channel, add, mode, param);
}

void Protocol::callUserModeHook(std::string nick, bool add, std::string mode) {
	bot->modUserModeHook(serverName, nick, add, mode);
}

void Protocol::callUserSNOMaskHook(std::string nick, bool add, char snomask) {
	bot->modUserSNOMaskHook(serverName, nick, add, snomask);
}

void Protocol::callChanTopicHook(std::string channel, std::string topic) {
	bot->modChanTopicHook(serverName, channel, topic);
}

void Protocol::callChanJoinHook(std::string channel, std::string nick) {
	bot->modChanJoinHook(serverName, channel, nick);
}

void Protocol::callChanPartHook(std::string channel, std::string nick, std::string reason) {
	bot->modChanPartHook(serverName, channel, nick, reason);
}

void Protocol::callChanKickHook(std::string channel, std::string kicker, std::string kickee, std::string reason) {
	bot->modChanKickHook(serverName, channel, kicker, kickee, reason);
}

void Protocol::callUserNickHook(std::string oldNick, std::string newNick) {
	bot->modUserNickHook(serverName, oldNick, newNick);
}

void Protocol::callUserConnectHook(std::string nick) {
	bot->modUserConnectHook(serverName, nick);
}

void Protocol::callUserQuitHook(std::string nick, std::string reason) {
	bot->modUserQuitHook(serverName, nick, reason);
}

void Protocol::callNumericHook(std::string client, std::string numeric, std::vector<std::string> data) {
	bot->modNumericHook(serverName, client, numeric, data);
}

void Protocol::callOperHook(std::string nick, std::string operType) {
	bot->modOperHook(serverName, nick, operType);
}

void Protocol::callSNoticeHook(char snotype, std::string message) {
	bot->modSNoticeHook(serverName, snotype, message);
}

void Protocol::callMetadataHook(std::string target, std::string dataKey, std::string dataValue) {
	bot->modMetadataHook(serverName, target, dataKey, dataValue);
}

void Protocol::callXLineAddHook(std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {
	bot->modXLineAddHook(serverName, lineType, mask, setter, expiry, reason);
}

void Protocol::callXLineRemoveHook(std::string lineType, std::string mask) {
	bot->modXLineRemoveHook(serverName, lineType, mask);
}

void Protocol::callServerConnectHook(std::string newServerName) {
	bot->modServerConnectHook(serverName, newServerName);
}

void Protocol::callServerQuitHook(std::string quitServerName, std::string reason) {
	bot->modServerQuitHook(serverName, quitServerName, reason);
}

void Protocol::callOtherDataHook(std::string client, std::vector<std::string> lineTokens) {
	bot->modOtherDataHook(serverName, client, lineTokens);
}

void Protocol::callChanMsgOutHook(std::string client, std::string channel, char status, std::string message) {
	bot->modChanMsgOutHook(serverName, client, channel, status, message);
}

void Protocol::callChanMsgSendHook(std::string client, std::string channel, char status, std::string message) {
	bot->modChanMsgSendHook(serverName, client, channel, status, message);
}

void Protocol::callUserMsgOutHook(std::string client, std::string nick, std::string message) {
	bot->modUserMsgOutHook(serverName, client, nick, message);
}

void Protocol::callUserMsgSendHook(std::string client, std::string nick, std::string message) {
	bot->modUserMsgSendHook(serverName, client, nick, message);
}

void Protocol::callChanNoticeOutHook(std::string client, std::string channel, char status, std::string message) {
	bot->modChanNoticeOutHook(serverName, client, channel, status, message);
}

void Protocol::callChanNoticeSendHook(std::string client, std::string channel, char status, std::string message) {
	bot->modChanNoticeSendHook(serverName, client, channel, status, message);
}

void Protocol::callUserNoticeOutHook(std::string client, std::string nick, std::string message) {
	bot->modUserNoticeOutHook(serverName, client, nick, message);
}

void Protocol::callUserNoticeSendHook(std::string client, std::string nick, std::string message) {
	bot->modUserNoticeSendHook(serverName, client, nick, message);
}

void Protocol::callChanCTCPOutHook(std::string client, std::string channel, char status, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPOutHook(serverName, client, channel, status, ctcp, "");
	else
		bot->modChanCTCPOutHook(serverName, client, channel, status, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callChanCTCPSendHook(std::string client, std::string channel, char status, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPSendHook(serverName, client, channel, status, ctcp, "");
	else
		bot->modChanCTCPSendHook(serverName, client, channel, status, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPOutHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPOutHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPOutHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPSendHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPSendHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPSendHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callChanCTCPReplyOutHook(std::string client, std::string channel, char status, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPReplyOutHook(serverName, client, channel, status, ctcp, "");
	else
		bot->modChanCTCPReplyOutHook(serverName, client, channel, status, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callChanCTCPReplySendHook(std::string client, std::string channel, char status, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modChanCTCPReplySendHook(serverName, client, channel, status, ctcp, "");
	else
		bot->modChanCTCPReplySendHook(serverName, client, channel, status, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPReplyOutHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPReplyOutHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPReplyOutHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}

void Protocol::callUserCTCPReplySendHook(std::string client, std::string nick, std::string ctcp) {
	size_t spacePos = ctcp.find_first_of(' ');
	if (spacePos == std::string::npos)
		bot->modUserCTCPReplySendHook(serverName, client, nick, ctcp, "");
	else
		bot->modUserCTCPReplySendHook(serverName, client, nick, ctcp.substr(0, spacePos), ctcp.substr(spacePos + 1));
}