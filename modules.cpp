#include "modules.h"

#ifndef MODBASE_ROBOBO
#define MODBASE_ROBOBO
class Module {
	public:
		void init(std::tr1::unordered_map<std::string, Module>* moduleList, std::list<std::string>* serverList, ModuleInterface* modFace);
		Module();
		virtual ~Module();
		virtual void onChannelMsg(std::string channel, char target, std::string nick, std::string message);
		virtual void onUserMsg(std::string nick, std::string message);
		virtual void onChannelNotice(std::string channel, char target, std::string nick, std::string message);
		virtual void onUserNotice(std::string nick, std::string message);
		virtual void onChannelCTCP(std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCP(std::string nick, std::string message);
		virtual void onChannelCTCPReply(std::string channel, char target, std::string nick, std::string message);
		virtual void onUserCTCPReply(std::string nick, std::string message);
		virtual void onChannelJoin(std::string channel, std::string hostmask);
		virtual void onChannelPart(std::string channel, std::string hostmask, std::string reason);
		virtual void onUserQuit(std::string hostmask, std::string reason);
		virtual void onChannelKick(std::string channel, std::string kicker, std::string kickee, std::string reason);
		virtual void onNumeric(std::string numeric, std::string message);
		virtual void onOutMessage(std::string target, std::string message);
		virtual void onOutNotice(std::string target, std::string message);
	protected:
		std::string moduleName;
		std::tr1::unordered_map<std::string, Module>* modules;
		std::list<std::string>* servers;
		ModuleInterface* serverData;
		void sendPrivMsg(std::string server, std::string target, std::string message);
		void sendNotice(std::string server, std::string target, std::string message);
		void sendCTCP(std::string server, std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string server, std::string target, std::string type, std::string data = "");
		void joinChannel(std::string server, std::string channel, std::string key = "");
		void partChannel(std::string server, std::string channel, std::string reason);
		void kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason);
		std::vector<std::string> splitBySpace(std::string line);
};

void Module::init(std::tr1::unordered_map<std::string, Module>* moduleList, std::list<std::string>* serverList, ModuleInterface* modFace) {
	modules = moduleList;
	servers = serverList;
	serverData = modFace;
}

Module::Module() {}

Module::~Module() {}

void Module::onChannelMsg(std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserMsg(std::string nick, std::string message) {}

void Module::onChannelNotice(std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserNotice(std::string nick, std::string message) {}

void Module::onChannelCTCP(std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCP(std::string nick, std::string message) {}

void Module::onChannelCTCPReply(std::string channel, char target, std::string nick, std::string message) {}

void Module::onUserCTCPReply(std::string nick, std::string message) {}

void Module::onChannelJoin(std::string channel, std::string hostmask) {}

void Module::onChannelPart(std::string channel, std::string hostmask, std::string reason) {}

void Module::onUserQuit(std::string hostmask, std::string reason) {}

void Module::onChannelKick(std::string channel, std::string kicker, std::string kickee, std::string reason) {}

void Module::onNumeric(std::string numeric, std::string message) {}

void Module::onOutMessage(std::string target, std::string message) {}

void Module::onOutNotice(std::string target, std::string message) {}

/*void Module::sendPrivMsg(std::string server, std::string target, std::string message) {
	bot_socket->sendMsg("PRIVMSG " + target + " :" + message);
}

void Module::sendNotice(std::string server, std::string target, std::string message) {
	bot_socket->sendMsg("NOTICE " + target + " :" + message);
}

void Module::sendCTCP(std::string server, std::string target, std::string type, std::string params) {
	if (params != "")
		bot_socket->sendMsg("PRIVMSG " + target + " :" + (char)1 + type + " " + params + (char)1);
	else
		bot_socket->sendMsg("PRIVMSG " + target + " :" + (char)1 + type + (char)1);
}

void Module::sendCTCPReply(std::string server, std::string target, std::string type, std::string data) {
	if (data != "")
		bot_socket->sendMsg("NOTICE " + target + " :" + (char)1 + type + " " + data + (char)1);
	else
		bot_socket->sendMsg("NOTICE " + target + " :" + (char)1 + type + (char)1);
}

void Module::joinChannel(std::string server, std::string channel, std::string key) {
	bot_socket->sendMsg("JOIN " + channel + " " + key);
}

void Module::partChannel(std::string server, std::string channel, std::string reason) {
	bot_socket->sendMsg("PART " + channel + " :" + reason);
}

void Module::kickChannelUser(std::string server, std::string channel, std::string nick, std::string reason) {
	bot_socket->sendMsg("KICK " + channel + " " + nick + " :" + reason);
} */

std::vector<std::string> splitBySpace(std::string line) {
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