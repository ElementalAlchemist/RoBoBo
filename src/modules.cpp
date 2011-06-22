#include "modules.h"

Module::Module(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : moduleName(modName), baseDir(dir), debugLevel(debug), config(modConf), serverData(modFace) {}

Module::~Module() {}

void Module::reconf(std::tr1::unordered_map<std::string, std::string> modConf) {
	config = modConf;
}

Priority Module::receivePriority() { return PRI_NORMAL; }

bool Module::onLoadComplete() { return true; }

void Module::onRehash() {}

void Module::onModuleChange() {}

bool Module::onChannelMsg(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) { return true; }

bool Module::onUserMsg(std::string server, std::string client, std::string nick, std::string message) { return true; }

bool Module::onChannelNotice(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) { return true; }

bool Module::onUserNotice(std::string server, std::string client, std::string nick, std::string message) { return true; }

bool Module::onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) { return true; }

bool Module::onUserCTCP(std::string server, std::string client, std::string nick, std::string message) { return true; }

bool Module::onChannelCTCPReply(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) { return true; }

bool Module::onUserCTCPReply(std::string server, std::string client, std::string nick, std::string message) { return true; }

void Module::onChannelJoinPre(std::string server, std::string client, std::string channel, std::string hostmask) {}

void Module::onChannelJoinPost(std::string server, std::string channel, std::string hostmask) {}

void Module::onChannelPartPre(std::string server, std::string channel, std::string hostmask, std::string reason) {}

void Module::onChannelPartPost(std::string server, std::string channel, std::string hostmask, std::string reason) {}

void Module::onUserConnectPre(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {}

void Module::onUserConnectPost(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {}

void Module::onUserQuitPre(std::string server, std::string hostmask, std::string reason) {}

void Module::onUserQuitPost(std::string server, std::string hostmask, std::string reason) {}

void Module::onNickChangePre(std::string server, std::string oldNick, std::string newNick) {}

void Module::onNickChangePost(std::string server, std::string oldNick, std::string newNick) {}

void Module::onChannelKickPre(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onChannelKickPost(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onChannelModePre(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param) {}

void Module::onChannelModePost(std::string server, std::string channel, std::string setter, std::string mode, bool add, std::string param) {}

void Module::onUserModePre(std::string server, std::string client, std::string mode, bool add) {}

void Module::onUserModePost(std::string server, std::string client, std::string mode, bool add) {}

void Module::onUserOperPre(std::string server, std::string user, std::string opertype) {}

void Module::onUserOperPost(std::string server, std::string user, std::string opertype) {}

void Module::onNumeric(std::string server, std::string client, std::string numeric, std::vector<std::string> parsedLine) {}

void Module::onOtherData(std::string server, std::string client, std::vector<std::string> parsedLine) {}

void Module::onPreConnect(std::string server) {}

void Module::onConnect(std::string server, std::string client) {}

void Module::onQuit(std::string server, std::string client) {}

std::string Module::onChannelMessageOut(std::string server, std::string client, std::string target, char status, std::string message) { return message; }

void Module::onChannelMessageSend(std::string server, std::string client, std::string target, char status, std::string message) {}

std::string Module::onUserMessageOut(std::string server, std::string client, std::string target, std::string message) { return message; }

void Module::onUserMessageSend(std::string server, std::string client, std::string target, std::string message) {}

std::string Module::onChannelNoticeOut(std::string server, std::string client, std::string target, char status, std::string message) { return message; }

void Module::onChannelNoticeSend(std::string server, std::string client, std::string target, char status, std::string message) {}

std::string Module::onUserNoticeOut(std::string server, std::string client, std::string target, std::string message) { return message; }

void Module::onUserNoticeSend(std::string server, std::string client, std::string target, std::string message) {}

std::string Module::onChannelCTCPOut(std::string server, std::string client, std::string target, char status, std::string message) { return message; }

void Module::onChannelCTCPSend(std::string server, std::string client, std::string target, char status, std::string message) {}

std::string Module::onUserCTCPOut(std::string server, std::string client, std::string target, std::string message) { return message; }

void Module::onUserCTCPSend(std::string server, std::string client, std::string target, std::string message) {}

std::string Module::onChannelCTCPReplyOut(std::string server, std::string client, std::string target, char status, std::string message) { return message; }

void Module::onChannelCTCPReplySend(std::string server, std::string client, std::string target, char status, std::string message) {}

std::string Module::onUserCTCPReplyOut(std::string server, std::string client, std::string target, std::string message) { return message; }

void Module::onUserCTCPReplySend(std::string server, std::string client, std::string target, std::string message) {}

std::string Module::description() { return "This module has not been described."; }

std::vector<std::string> Module::abilities() { return std::vector<std::string> (); }

std::vector<std::string> Module::supports() { return std::vector<std::string> (); }

void Module::sendPrivMsg(std::string server, std::string client, std::string target, std::string message) {
	serverData->sendPrivMsg(server, client, target, message);
}

void Module::sendNotice(std::string server, std::string client, std::string target, std::string message) {
	serverData->sendNotice(server, client, target, message);
}

void Module::sendCTCP(std::string server, std::string client, std::string target, std::string type, std::string params) {
	if (params == "")
		serverData->sendPrivMsg(server, client, target, (char)1 + type + (char)1);
	else
		serverData->sendPrivMsg(server, client, target, (char)1 + type + " " + params + (char)1);
}

void Module::sendCTCPReply(std::string server, std::string client, std::string target, std::string type, std::string data) {
	if (data == "")
		serverData->sendNotice(server, client, target, (char)1 + type + (char)1);
	else
		serverData->sendNotice(server, client, target, (char)1 + type + " " + data + (char)1);
}

void Module::joinChannel(std::string server, std::string client, std::string channel, std::string key) {
	serverData->joinChannel(server, client, channel, key);
}

void Module::partChannel(std::string server, std::string client, std::string channel, std::string reason) {
	serverData->partChannel(server, client, channel, reason);
}

void Module::kickChannelUser(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {
	serverData->kickUser(server, client, channel, nick, reason);
}

void Module::setMode(std::string server, std::string client, std::string channel, std::string mode) {
	serverData->setMode(server, client, channel, mode);
}

void Module::removeMode(std::string server, std::string client, std::string channel, std::string mode) {
	serverData->removeMode(server, client, channel, mode);
}

void Module::changeNick(std::string server, std::string client, std::string nick) {
	serverData->changeNick(server, client, nick);
}

void Module::oper(std::string server, std::string client, std::string username, std::string password) {
	serverData->oper(server, client, username, password);
}

void Module::killUser(std::string server, std::string client, std::string user, std::string reason) {
	serverData->killUser(server, client, user, reason);
}

void Module::setXLine(std::string server, std::string client, char lineType, std::string hostmask, time_t duration, std::string reason) {
	serverData->setXLine(server, client, lineType, hostmask, duration, reason);
}

void Module::removeXLine(std::string server, std::string client, char lineType, std::string hostmask) {
	serverData->removeXLine(server, client, lineType, hostmask);
}

std::tr1::unordered_map<char, std::tr1::unordered_map<std::string, time_t> > Module::listXLines(std::string server) {
	return serverData->listXLines(server);
}

void Module::sendSNotice(std::string server, char snomask, std::string text) {
	serverData->sendSNotice(server, snomask, text);
}

void Module::sendOtherCommand(std::string server, std::string command, std::string data) {
	serverData->sendOther(server, command + " " + data);
}

std::set<std::string> Module::clients(std::string server) {
	return serverData->clients(server);
}

std::tr1::unordered_map<std::string, std::string> Module::clientInfo(std::string server, std::string client) {
	return serverData->clientInfo(server, client);
}

std::string Module::addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	return serverData->addClient(server, nick, ident, host, gecos);
}

void Module::removeClient(std::string server, std::string client, std::string reason) {
	serverData->removeClient(server, client, reason);
}

std::set<std::string> Module::clients(std::string server) {
	return serverData->clients(server);
}

std::tr1::unordered_map<std::string, std::string> Module::clientInfo(std::string server, std::string client) {
	return serverData->clientInfo(server, client);
}

bool Module::connectServer(std::string server) {
	return serverData->connectServer(server);
}

void Module::quitServer(std::string server, std::string reason) {
	serverData->quitServer(server, reason);
}

bool Module::loadModule(std::string modName) {
	return serverData->loadModule(modName, false);
}

void Module::unloadModule() {
	serverData->unloadModule(moduleName);
}

void Module::unloadModule(std::string modName) {
	serverData->unloadModule(modName);
}

void Module::rehashBot() {
	serverData->rehash();
}

unsigned int Module::botVersion() {
	return 1900; // 2.0.0-devel
}

std::vector<std::string> Module::splitHostmask(std::string hostmask) {
	std::vector<std::string> splitmask;
	int exclamationPoint = hostmask.find_first_of('!');
	int atSign = hostmask.find_first_of('@');
	splitmask.push_back(hostmask.substr(0, exclamationPoint));
	splitmask.push_back(hostmask.substr(exclamationPoint, atSign - exclamationPoint));
	splitmask.push_back(hostmask.substr(atSign));
	return splitmask;
}

std::vector<std::string> Module::splitBySpace(std::string line) {
	std::vector<std::string> split;
	std::string temp = "";
	for (unsigned int i = 0; i < line.size(); i++) {
		if (line[i] == ' ') {
			split.push_back(temp);
			temp = "";
		} else
			temp += line[i];
	}
	if (temp != "")
		split.push_back(temp);
	return split;
}

std::string Module::stripColors(std::string message) {
	while (message.find_first_of((char)2) != std::string::npos)
		message.erase(message.find_first_of((char)2), 1);
	while (message.find_first_of((char)31) != std::string::npos)
		message.erase(message.find_first_of((char)31), 1);
	while (message.find_first_of((char)3) != std::string::npos) {
		size_t colorpos = message.find_first_of((char)3);
		std::string afterColor = message.substr(colorpos, 6);
		if (afterColor.size() == 1) { // afterColor's size could be less than 6 if the color char is close to the end of the string
			message.erase(colorpos, 1);
			break;
		}
		bool comma = false;
		unsigned int i;
		for (i = 1; i < afterColor.size(); i++) {
			if (afterColor[i] == '0' || afterColor[i] == '1' || afterColor[i] == '2' || afterColor[i] == '3' || afterColor[i] == '4' || afterColor[i] == '5' || afterColor[i] == '6' || afterColor[i] == '7' || afterColor[i] == '8' || afterColor[i] == '9')
				continue;
			if (afterColor[i] == ',' && !comma) {
				comma = true;
				continue;
			}
			break;
		}
		message.erase(colorpos, i);
	}
	return message;
}

std::list<std::string> Module::servers() {
	return serverData->serverList();
}

std::tr1::unordered_map<std::string, Module*> Module::modules() {
	return serverData->loadedModules();
}

std::multimap<std::string, std::string> Module::modAbilities() {
	return serverData->moduleAbilities();
}

std::tr1::unordered_map<std::string, std::vector<std::string> > Module::modSupports() {
	return serverData->moduleSupports();
}

bool Module::serverIsClient(std::string server) {
	return serverData->serverIsClient(server);
}

std::tr1::unordered_map<std::string, std::string> Module::serverInfo(std::string server) {
	return serverData->serverData(server);
}

std::vector<std::vector<std::string> > Module::serverChanModes(std::string server) {
	return serverData->serverChanModes(server);
}

std::list<std::pair<std::string, char> > Module::serverPrefixes(std::string server) {
	return serverData->serverPrefixes(server);
}

std::list<std::string> Module::channels(std::string server) {
	return serverData->channels(server);
}

std::string Module::channelTopic(std::string server, std::string channel) {
	return serverData->channelTopic(server, channel);
}

std::set<std::string> Module::channelUsers(std::string server, std::string channel) {
	return serverData->channelUsers(server, channel);
}

std::string Module::userIdent(std::string server, std::string user) {
	return serverData->userIdent(server, user);
}

std::string Module::userHost(std::string server, std::string user) {
	return serverData->userHost(server, user);
}

std::list<std::string> Module::userModes(std::string server, std::string user) {
	return serverData->userModes(server, user);
}

std::pair<std::string, char> Module::userStatus(std::string server, std::string channel, std::string user) {
	return serverData->userStatus(server, channel, user);
}