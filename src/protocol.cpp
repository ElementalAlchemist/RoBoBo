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

void Protocol::sendPrivMsg(std::string client, std::string target, std::string message) {}

void Protocol::sendNotice(std::string client, std::string target, std::string message) {}

void Protocol::sendCTCP(std::string client, std::string target, std::string ctcp) {}

void Protocol::sendCTCPReply(std::string client, std::string target, std::string ctcp) {}

void Protocol::setMode(std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes) {}

void Protocol::setSNOMask(std::string client, char snomask, bool add) {}

void Protocol::setChanTopic(std::string client, std::string channel, std::string topic) {}

void Protocol::joinChannel(std::string client, std::string channel, std::string key) {}

void Protocol::partChannel(std::string client, std::string channel, std::string reason) {}

std::string Protocol::addClient(std::string nick, std::string ident, std::string host, std::string gecos) {
	return "";
}

void Protocol::removeClient(std::string client) {}

void Protocol::changeNick(std::string client, std::string newNick) {}

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

void Protocol::callChanModeHook(std::string client, std::string channel, bool add, std::string mode, std::string param) {
	bot->modChanModeHook(serverName, client, channel, add, mode, param);
}

void Protocol::callUserModeHook(std::string client, bool add, std::string mode) {
	bot->modUserModeHook(serverName, client, add, mode);
}

void Protocol::callUserSNOMaskHook(std::string client, bool add, char snomask) {
	bot->modUserSNOMaskHook(serverName, client, add, snomask);
}

void Protocol::callChanTopicHook(std::string client, std::string channel, std::string topic) {
	bot->modChanTopicHook(serverName, client, channel, topic);
}

void Protocol::callChanJoinHook(std::string client, std::string channel, std::string nick) {
	bot->modChanJoinHook(serverName, client, channel, nick);
}

void Protocol::callChanPartHook(std::string client, std::string channel, std::string nick, std::string reason) {
	bot->modChanPartHook(serverName, client, channel, nick, reason);
}

void Protocol::callUserConnectHook(std::string nick) {
	bot->modUserConnectHook(serverName, nick);
}

void Protocol::callUserQuitHook(std::string client, std::string nick, std::string reason) {
	bot->modUserQuitHook(serverName, client, nick, reason);
}

void Protocol::callUserNickHook(std::string client, std::string oldNick, std::string newNick) {
	bot->modUserNickHook(serverName, client, oldNick, newNick);
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