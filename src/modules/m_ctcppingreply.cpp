#include "modinclude.h"

class PingReply : public Module {
	public:
		int botAPIversion();
		bool onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		bool onUserCTCP(std::string server, std::string nick, std::string message);
		std::string description();
};

int PingReply::botAPIversion() {
	return 2000;
}

bool PingReply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, nick, "PING", "");
	}
	return true;
}

bool PingReply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, nick, "PING", "");
	}
	return true;
}

std::string PingReply::description() {
	return "Responds to CTCP PING requests.";
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, unsigned short debug) {
	return new PingReply (modConf, modFace, modName, debug);
}