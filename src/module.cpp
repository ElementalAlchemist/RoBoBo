#include "module.h"

Module::Module(std::string modName, std::map<std::string, std::string> conf, std::string workDir, unsigned short debug, Base* botptr) : moduleName(modName), workingDir(workDir), config(conf), debugLevel(debug), bot(botptr), priority(PRI_NORMAL) {}
// The default priority is PRI_NORMAL but can be overridden in the subclass's constructor

Module::~Module() {}
// This is just here because we need a virtual destructor.

bool Module::onLoadComplete() {
	return true;
}

void Module::onUnload() {}

void Module::onRehash() {}

void Module::onModuleLoad(std::string modName) {}

void Module::onModuleUnload(std::string modName) {}

bool Module::forceKeepAlive() {
	return false;
}

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

void Module::onChanMode(std::string server, std::string client, std::string channel, std::string setter, bool add, std::string mode, std::string param) {}

void Module::onUserMode(std::string server, std::string client, bool add, std::string mode) {}

void Module::onUserSNOMask(std::string server, std::string client, bool add, char snomask) {}

void Module::onChanTopic(std::string server, std::string client, std::string channel, std::string setter, std::string topic) {}

void Module::onChanJoin(std::string server, std::string client, std::string channel, std::string nick) {}

void Module::onChanPart(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {}

void Module::onChanKick(std::string server, std::string client, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onUserConnect(std::string server, std::string nick) {}

void Module::onUserQuit(std::string server, std::string client, std::string nick, std::string reason) {}

void Module::onUserNick(std::string server, std::string client, std::string oldNick, std::string newNick) {}

void Module::onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {}

void Module::onOper(std::string server, std::string nick, std::string operType) {}

void Module::onSNotice(std::string server, char snotype, std::string message) {}

void Module::onMetadata(std::string server, std::string target, std::string dataKey, std::string dataValue) {}

void Module::onXLineAdd(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {}

void Module::onXLineRemove(std::string server, std::string lineType, std::string mask) {}

void Module::onServerConnect(std::string server, std::string newServerName) {}

void Module::onServerQuit(std::string server, std::string quitServerName, std::string reason) {}

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
	return "A description has not been provided by the module author.";
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

void Module::sendPrivMsg(std::string server, std::string client, std::string target, std::string message) {
	bot->sendPrivMsg(server, client, target, message);
}

void Module::sendNotice(std::string server, std::string client, std::string target, std::string message) {
	bot->sendNotice(server, client, target, message);
}

void Module::sendCTCP(std::string server, std::string client, std::string target, std::string ctcp, std::string params) {
	bot->sendCTCP(server, client, target, ctcp, params);
}

void Module::sendCTCPReply(std::string server, std::string client, std::string target, std::string ctcp, std::string params) {
	bot->sendCTCPReply(server, client, target, ctcp, params);
}

void Module::setMode(std::string server, std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes) {
	bot->setMode(server, client, target, setModes, delModes);
}

void Module::setSNOMask(std::string server, std::string client, char snomask, bool add) {
	bot->setSNOMask(server, client, snomask, add);
}

void Module::setChanTopic(std::string server, std::string client, std::string channel, std::string topic) {
	bot->setChanTopic(server, client, channel, topic);
}

void Module::joinChannel(std::string server, std::string client, std::string channel, std::string key) {
	bot->joinChannel(server, client, channel, key);
}

void Module::partChannel(std::string server, std::string client, std::string channel, std::string reason) {
	bot->partChannel(server, client, channel, reason);
}

void Module::changeNick(std::string server, std::string client, std::string newNick) {
	bot->changeNick(server, client, newNick);
}

void Module::kickUser(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {
	bot->kickUser(server, client, channel, nick, reason);
}

void Module::connectServer(std::string server) {
	bot->connectServer(server);
}

std::string Module::addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	return bot->addClient(server, nick, ident, host, gecos);
}

void Module::removeClient(std::string server, std::string client) {
	bot->removeClient(server, client);
}

void Module::quitServer(std::string server) {
	bot->disconnectServer(server);
}

void Module::oper(std::string server, std::string client, std::string username, std::string password) {
	bot->oper(server, client, username, password);
}

void Module::sendSNotice(std::string server, char snomask, std::string message) {
	bot->sendSNotice(server, snomask, message);
}

void Module::setMetadata(std::string server, std::string target, std::string key, std::string value) {
	bot->setMetadata(server, target, key, value);
}

void Module::setXLine(std::string server, std::string client, std::string linetype, std::string mask, time_t duration, std::string reason) {
	bot->setXLine(server, client, linetype, mask, duration, reason);
}

void Module::delXLine(std::string server, std::string client, std::string linetype, std::string mask) {
	bot->delXLine(server, client, linetype, mask);
}

void Module::sendOtherData(std::string server, std::string client, std::string line) {
	bot->sendOtherData(server, client, line);
}