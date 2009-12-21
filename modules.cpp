#include "modules.h"

#ifndef MODBASE_ROBOBO
#define MODBASE_ROBOBO
class Module {
	public:
		void init(std::tr1::unordered_map<std::string, Module>* moduleList, std::list<std::string>* serverList, ModuleInterface* modFace);
		Module();
		virtual ~Module();
		virtual void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserMsg(std::string server, std::string nick, std::string message);
		virtual void onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserNotice(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCP(std::string server, std::string nick, std::string message);
		virtual void onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCPReply(std::string server, std::string nick, std::string message);
		virtual void onChannelJoin(std::string server, std::string channel, std::string hostmask);
		virtual void onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason);
		virtual void onUserQuit(std::string server, std::string hostmask, std::string reason);
		virtual void onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param);
		virtual void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		virtual void onOtherData(std::string server, std::vector<std::string> parsedLine);
		
		virtual void onOutChannelMessage(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserMessage(std::string server, std::string target, std::string message);
		virtual void onOutChannelNotice(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserNotice(std::string server, std::string target, std::string message);
		virtual void onOutChannelCTCP(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserCTCP(std::string server, std::string target, std::string message);
		virtual void onOutChannelCTCPReply(std::string server, std::string target, char status, std::string message);
		virtual void onOutUserCTCPReply(std::string server, std::string target, std::string message);
	protected:
		std::string moduleName;
		std::tr1::unordered_map<std::string, Module>* modules;
		std::list<std::string>* servers;
		void sendPrivMsg(std::string server, std::string target, std::string message);
		void sendNotice(std::string server, std::string target, std::string message);
		void sendCTCP(std::string server, std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string server, std::string target, std::string type, std::string data = "");
		void joinChannel(std::string server, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string channel, std::string reason);
		void kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason);
		void setMode(std::string server, std::string channel, char mode, bool add, std::string param = "");
		void quitServer(std::string server, std::string reason);
		std::vector<std::string> splitHostmask(std::string hostmask);
		std::vector<std::string> splitBySpace(std::string line);
	private:
		ModuleInterface* serverData;
};

void Module::init(std::tr1::unordered_map<std::string, Module>* moduleList, std::list<std::string>* serverList, ModuleInterface* modFace) {
	modules = moduleList;
	servers = serverList;
	serverData = modFace;
}

Module::Module() {}

Module::~Module() {}

void Module::onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserMsg(std::string server, std::string nick, std::string message) {}

void Module::onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserNotice(std::string server, std::string nick, std::string message) {}

void Module::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCP(std::string server, std::string nick, std::string message) {}

void Module::onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCPReply(std::string server, std::string nick, std::string message) {}

void Module::onChannelJoin(std::string server, std::string channel, std::string hostmask) {}

void Module::onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason) {}

void Module::onUserQuit(std::string server, std::string hostmask, std::string reason) {}

void Module::onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param) {}

void Module::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {}

void Module::onOtherData(std::string server, std::vector<std::string> parsedLine) {}

void Module::onOutChannelMessage(std::string server, std::string target, char status, std::string message) {}

void Module::onOutUserMessage(std::string server, std::string target, std::string message) {}

void Module::onOutChannelNotice(std::string server, std::string target, char status, std::string message) {}

void Module::onOutUserNotice(std::string server, std::string target, std::string message) {}

void Module::onOutChannelCTCP(std::string server, std::string target, char status, std::string message) {}

void Module::onOutUserCTCP(std::string server, std::string target, std::string message) {}

void Module::onOutChannelCTCPReply(std::string server, std::string target, char status, std::string message) {}

void Module::onOutUserCTCPReply(std::string server, std::string target, std::string message) {}

void Module::sendPrivMsg(std::string server, std::string target, std::string message) {
	serverData.sendToServer(server, "PRIVMSG " + target + " :" + message);
}

void Module::sendNotice(std::string server, std::string target, std::string message) {
	serverData.sendToServer(server, "NOTICE " + target + " :" + message);
}

void Module::sendCTCP(std::string server, std::string target, std::string type, std::string params = "") {
	if (params == "")
		serverData.sendToServer(server, "PRIVMSG " + target + " :" + (char)1 + type + (char)1);
	else
		serverData.sendToServer(server, "PRIVMSG " + target + " :" + (char)1 + type + " " + params + (char)1);
}

void Module::sendCTCPReply(std::string server, std::string target, std::string type, std::string data = "") {
	if (data == "")
		serverData.sendToServer(server, "NOTICE " + target + " :" + (char)1 + type + (char)1);
	else
		serverData.sendToServer(server, "NOTICE " + target + " :" + (char)1 + type + " " + data + (char)1);
}

void Module::joinChannel(std::string server, std::string channel, std::string key = "") {
	serverData.sendToServer(server, "JOIN " + channel + " " + key);
}

void Module::partChannel(std::string server, std::string channel, std::string reason) {
	serverData.sendToServer(server, "PART " + channel + " :" + reason);
}

void Module::kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason) {
	serverData.sendToServer(server, "KICK " + channel + " " + nick + " :" + reason);
}

void Module::setMode(std::string server, std::string channel, char mode, bool add, std::string param = "") {
	if (param == "") {
		if (add)
			serverData.sendToServer(server, "MODE " + channel + " +" + mode);
		else
			serverData.sendToServer(server, "MODE " + channel + " -" + mode);
	} else {
		if (add)
			serverData.sendToServer(server, "MODE " + channel + " +" + mode + " " + param);
		else
			serverData.sendToServer(server, "MODE " + channel + " -" + mode + " " + param);
	}
}

void Module::quitServer(std::string server, std::string reason) {
	serverData.sendToServer(server, "QUIT :" + reason);
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
	return split;
}
#endif