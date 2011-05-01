#include "modules.h"

Module::Module(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, unsigned short debug) : config(modConf), serverData(modFace), moduleName(modName), debugLevel(debug) {}

Module::~Module() {}

void Module::reconf(std::tr1::unordered_map<std::string, std::string> modConf) {
	config = modConf;
}

Priority Module::receivePriority() { return NORMAL; }

bool Module::onLoadComplete() { return true; }

void Module::onRehash() {}

void Module::onModuleChange() {}

void Module::onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserMsg(std::string server, std::string nick, std::string message) {}

void Module::onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserNotice(std::string server, std::string nick, std::string message) {}

void Module::onChannelAction(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserAction(std::string server, std::string nick, std::string message) {}

void Module::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCP(std::string server, std::string nick, std::string message) {}

void Module::onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCPReply(std::string server, std::string nick, std::string message) {}

void Module::onChannelJoinPre(std::string server, std::string channel, std::string hostmask) {}

void Module::onChannelJoinPost(std::string server, std::string channel, std::string hostmask) {}

void Module::onChannelPartPre(std::string server, std::string channel, std::string hostmask, std::string reason) {}

void Module::onChannelPartPost(std::string server, std::string channel, std::string hostmask, std::string reason) {}

void Module::onUserQuitPre(std::string server, std::string hostmask, std::string reason) {}

void Module::onUserQuitPost(std::string server, std::string hostmask, std::string reason) {}

void Module::onNickChangePre(std::string server, std::string oldNick, std::string newNick) {}

void Module::onNickChangePost(std::string server, std::string oldNick, std::string newNick) {}

void Module::onChannelKickPre(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onChannelKickPost(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onChannelModePre(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param) {}

void Module::onChannelModePost(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param) {}

void Module::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {}

void Module::onOtherData(std::string server, std::vector<std::string> parsedLine) {}

void Module::onPreConnect(std::string server) {}

void Module::onConnect(std::string server) {}

void Module::onQuit(std::string server) {}

std::string Module::onOutChannelMessage(std::string server, std::string target, char status, std::string message) { return message; }

void Module::onSendChannelMessage(std::string server, std::string target, char status, std::string message) {}

std::string Module::onOutUserMessage(std::string server, std::string target, std::string message) { return message; }

void Module::onSendUserMessage(std::string server, std::string target, std::string message) {}

std::string Module::onOutChannelNotice(std::string server, std::string target, char status, std::string message) { return message; }

void Module::onSendChannelNotice(std::string server, std::string target, char status, std::string message) {}

std::string Module::onOutUserNotice(std::string server, std::string target, std::string message) { return message; }

void Module::onSendUserNotice(std::string server, std::string target, std::string message) {}

std::string Module::onOutChannelAction(std::string server, std::string target, char status, std::string message) { return message; }

void Module::onSendChannelAction(std::string server, std::string target, char status, std::string message) {}

std::string Module::onOutUserAction(std::string server, std::string target, std::string message) { return message; }

void Module::onSendUserAction(std::string server, std::string target, std::string message) {}

std::string Module::onOutChannelCTCP(std::string server, std::string target, char status, std::string message) { return message; }

void Module::onSendChannelCTCP(std::string server, std::string target, char status, std::string message) {}

std::string Module::onOutUserCTCP(std::string server, std::string target, std::string message) { return message; }

void Module::onSendUserCTCP(std::string server, std::string target, std::string message) {}

std::string Module::onOutChannelCTCPReply(std::string server, std::string target, char status, std::string message) { return message; }

void Module::onSendChannelCTCPReply(std::string server, std::string target, char status, std::string message) {}

std::string Module::onOutUserCTCPReply(std::string server, std::string target, std::string message) { return message; }

void Module::onSendUserCTCPReply(std::string server, std::string target, std::string message) {}

std::string Module::description() { return "This module has not been described."; }

std::vector<std::string> Module::abilities() { return std::vector<std::string> (); }

std::vector<std::string> Module::supports() { return std::vector<std::string> (); }

void Module::sendPrivMsg(std::string server, std::string target, std::string message) {
	serverData->sendToServer(server, "PRIVMSG " + target + " :" + message);
}

void Module::sendNotice(std::string server, std::string target, std::string message) {
	serverData->sendToServer(server, "NOTICE " + target + " :" + message);
}

void Module::sendCTCP(std::string server, std::string target, std::string type, std::string params) {
	if (params == "")
		serverData->sendToServer(server, "PRIVMSG " + target + " :" + (char)1 + type + (char)1);
	else
		serverData->sendToServer(server, "PRIVMSG " + target + " :" + (char)1 + type + " " + params + (char)1);
}

void Module::sendCTCPReply(std::string server, std::string target, std::string type, std::string data) {
	if (data == "")
		serverData->sendToServer(server, "NOTICE " + target + " :" + (char)1 + type + (char)1);
	else
		serverData->sendToServer(server, "NOTICE " + target + " :" + (char)1 + type + " " + data + (char)1);
}

void Module::joinChannel(std::string server, std::string channel, std::string key) {
	serverData->sendToServer(server, "JOIN " + channel + " " + key);
}

void Module::partChannel(std::string server, std::string channel, std::string reason) {
	serverData->sendToServer(server, "PART " + channel + " :" + reason);
}

void Module::kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason) {
	serverData->sendToServer(server, "KICK " + channel + " " + nick + " :" + reason);
}

void Module::setMode(std::string server, std::string channel, char mode, bool add, std::string param) {
	if (param == "") {
		if (add)
			serverData->sendToServer(server, "MODE " + channel + " +" + mode);
		else
			serverData->sendToServer(server, "MODE " + channel + " -" + mode);
	} else {
		if (add)
			serverData->sendToServer(server, "MODE " + channel + " +" + mode + " " + param);
		else
			serverData->sendToServer(server, "MODE " + channel + " -" + mode + " " + param);
	}
}

void Module::sendOtherCommand(std::string server, std::string command, std::string data) {
	serverData->sendToServer(server, command + " " + data);
}

bool Module::connectServer(std::string server) {
	return serverData->connectServer(server);
}

void Module::quitServer(std::string server, std::string reason) {
	serverData->sendToServer(server, "QUIT :" + reason);
}

bool Module::loadModule(std::string modName) {
	return serverData->loadModule(modName, false);
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

std::tr1::unordered_map<std::string, std::string> Module::serverInfo(std::string server) {
	return serverData->serverData(server);
}

std::vector<std::vector<char> > Module::serverChanModes(std::string server) {
	return serverData->serverChanModes(server);
}

std::vector<std::pair<char, char> > Module::serverPrefixes(std::string server) {
	return serverData->serverPrefixes(server);
}

std::list<std::string> Module::channels(std::string server) {
	return serverData->channels(server);
}

std::string Module::channelTopic(std::string server, std::string channel) {
	return serverData->channelTopic(server, channel);
}

std::list<std::string> Module::channelUsers(std::string server, std::string channel) {
	return serverData->channelUsers(server, channel);
}

std::string Module::userIdent(std::string server, std::string channel, std::string user) {
	return serverData->userIdent(server, channel, user);
}

std::string Module::userHost(std::string server, std::string channel, std::string user) {
	return serverData->userHost(server, channel, user);
}

std::pair<char, char> Module::userStatus(std::string server, std::string channel, std::string user) {
	return serverData->userStatus(server, channel, user);
}