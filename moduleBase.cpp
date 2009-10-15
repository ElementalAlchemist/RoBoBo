#include "moduleBase.h"

class Module {
	public:
		Module(std::tr1::unordered_map<std::string, Channel>& channels, Socket& botSocket);
		virtual void onChannelMsg(std::string channel, char target, std::string nick, std::string message) = 0;
		virtual void onUserMsg(std::string nick, std::string message) = 0;
		virtual void onChannelNotice(std::string channel, char target, std::string nick, std::string message) = 0;
		virtual void onUserNotice(std::string nick, std::string message) = 0;
		virtual void onChannelCTCP(std::string channel, std::string nick, std::string type, std::string params) = 0;
		virtual void onUserCTCP(std::string nick, std::string type, std::string params) = 0;
		virtual void onChannelCTCPReply(std::string channel, std::string nick, std::string type, std::string data) = 0;
		virtual void onUserCTCPReply(std::string nick, std::string type, std::string data) = 0;
		virtual void onChannelJoin(std::string channel, std::string hostmask) = 0;
		virtual void onChannelPart(std::string channel, std::string hostmask, std::string reason) = 0;
		virtual void onUserQuit(std::string hostmask, std::string reason) = 0;
		virtual void onChannelKick(std::string channel, std::string kicker, std::string kickee, std::string reason) = 0;
		virtual void onOutMessage(std::string target, std::string message) = 0;
		virtual void onOutNotice(std::string target, std::string message) = 0;
	protected:
		std::tr1::unordered_map<std::string, Channel> *channelList;
		Socket* bot_socket;
		void sendPrivMsg(std::string target, std::string message);
		void sendNotice(std::string target, std::string message);
		void sendCTCP(std::string target, std::string type, std::string params = "");
		void sendCTCPReply(std::string target, std::string type, std::string data = "");
		void joinChannel(std::string channel, std::string key = "");
		void partChannel(std::string channel, std::string reason);
		void kickChannelUser(std::string channel, std::string nick, std::string reason);
		std::vector<std::string> splitBySpace(std::string line);
};

Module::Module(std::tr1::unordered_map<std::string, Channel>& channels, Socket& botSocket) {
	channelList = &channels;
	bot_socket = &botSocket;
}

/*void Module::sendPrivMsg(std::string target, std::string message) {
	bot_socket->sendMsg("PRIVMSG " + target + " :" + message);
}

void Module::sendNotice(std::string target, std::string message) {
	bot_socket->sendMsg("NOTICE " + target + " :" + message);
}

void Module::sendCTCP(std::string target, std::string type, std::string params) {
	if (params != "")
		bot_socket->sendMsg("PRIVMSG " + target + " :" + (char)1 + type + " " + params + (char)1);
	else
		bot_socket->sendMsg("PRIVMSG " + target + " :" + (char)1 + type + (char)1);
}

void Module::sendCTCPReply(std::string target, std::string type, std::string data) {
	if (data != "")
		bot_socket->sendMsg("NOTICE " + target + " :" + (char)1 + type + " " + data + (char)1);
	else
		bot_socket->sendMsg("NOTICE " + target + " :" + (char)1 + type + (char)1);
}

void Module::joinChannel(std::string channel, std::string key) {
	bot_socket->sendMsg("JOIN " + channel + " " + key);
}

void Module::partChannel(std::string channel, std::string reason) {
	bot_socket->sendMsg("PART " + channel + " :" + reason);
}

void Module::kickChannelUser(std::string channel, std::string nick, std::string reason) {
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