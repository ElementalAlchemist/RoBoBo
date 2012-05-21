#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), config(conf), workingDir(workDir), debugLevel(debug), bot(botptr), priority(PRI_NORMAL) {}
// The default priority is PRI_NORMAL but can be overridden in the subclass's constructor

bool Module::onLoadComplete() {
	return true;
}

void Module::onRehash() {}

void Module::onModuleLoad(std::string modName) {}

void Module::onModuleUnload(std::string modName) {}

void Module::rehash(std::map<std::string, std::string> conf) {
	config = conf;
}

void Module::endDebug() {
	debugLevel = 0;
}

MsgAction Module::onChanMsg(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	return MSG_CONTINUE;
}

MsgAction Module::onUserMsg(std::string server, std::string client, std::string nick, std::string message) {
	return MSG_CONTINUE;
}

MsgAction Module::onChanNotice(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	return MSG_CONTINUE;
}

MsgAction Module::onUserNotice(std::string server, std::string client, std::string nick, std::string message) {
	return MSG_CONTINUE;
}

MsgAction Module::onChanCTCP(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	return MSG_CONTINUE;
}

MsgAction Module::onUserCTCP(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	return MSG_CONTINUE;
}

MsgAction Module::onChanCTCPReply(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	return MSG_CONTINUE;
}

MsgAction Module::onUserCTCPReply(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	return MSG_CONTINUE;
}

void Module::onChanMode(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param) {}

void Module::onUserMode(std::string server, std::string client, bool add, std::string mode) {}

void Module::onUserSNOMask(std::string server, std::string client, bool add, char snomask) {}

void Module::onChanTopic(std::string server, std::string client, std::string channel, std::string topic) {}

void Module::onChanJoin(std::string server, std::string client, std::string channel, std::string nick) {}

void Module::onChanPart(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {}

void Module::onUserConnect(std::string server, std::string client, std::string nick) {}

void Module::onUserQuit(std::string server, std::string client, std::string nick, std::string reason) {}

void Module::onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {}

void Module::onOper(std::string server, std::string nick, std::string operType) {}

void Module::onSNotice(std::string server, std::string snotype, std::string message) {}

void Module::onUserMetadata(std::string server, std::string nick, std::string dataKey, std::string dataValue) {}

void Module::onServerData(std::string server, std::string dataType, std::string params) {}

void Module::onXLineAdd(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {}

void Module::onXLineRemove(std::string server, std::string lineType, std::string mask) {}

void Module::onOtherData(std::string server, std::string client, std::vector<std::string> lineTokens) {}

void Module::onChanMsgOut(std::string server, std::string client, std::string channel, char status, std::string &message) {}

void Module::onChanMsgSend(std::string server, std::string client, std::string channel, char status, std::string message) {}

void Module::onUserMsgOut(std::string server, std::string client, std::string nick, std::string &message) {}

void Module::onUserMsgSend(std::string server, std::string client, std::string nick, std::string message) {}

void Module::onChanNoticeOut(std::string server, std::string client, std::string channel, char status, std::string &message) {}

void Module::onChanNoticeSend(std::string server, std::string client, std::string channel, char status, std::string message) {}

void Module::onUserNoticeOut(std::string server, std::string client, std::string nick, std::string &message) {}

void Module::onUserNoticeSend(std::string server, std::string client, std::string nick, std::string message) {}

void Module::onChanCTCPOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {}

void Module::onChanCTCPSend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {}

void Module::onUserCTCPOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {}

void Module::onUserCTCPSend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {}

void Module::onChanCTCPReplyOut(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {}

void Module::onChanCTCPReplySend(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {}

void Module::onUserCTCPReplyOut(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {}

void Module::onUserCTCPReplySend(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {}

std::string Module::description() {
	return "A description has not been provided.";
}

std::list<std::string> Module::provides() {
	return std::list<std::string>();
}

std::list<std::string> Module::requires() {
	return std::list<std::string>();
}

std::list<std::string> Module::supports() {
	return std::list<std::string>();
}