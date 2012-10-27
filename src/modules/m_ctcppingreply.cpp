#include "modinclude.h"

class PingReply : public Module {
	public:
		PingReply(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserCTCP(std::string server, std::string client, std::string nick, std::string message);
		std::string description();
};

PingReply::PingReply(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

int PingReply::botAPIversion() {
	return 2001;
}

bool PingReply::onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	if (message.empty())
		return true;
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, client, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, client, nick, "PING", "");
	}
	return true;
}

bool PingReply::onUserCTCP(std::string server, std::string client, std::string nick, std::string message) {
	if (message.empty())
		return true;
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, client, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, client, nick, "PING", "");
	}
	return true;
}

std::string PingReply::description() {
	return "Responds to CTCP PING requests.";
}

MODULE_SPAWN(PingReply)