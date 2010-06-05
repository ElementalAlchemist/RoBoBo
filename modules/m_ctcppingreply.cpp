#include "modinclude.h"

class PingReply : public Module {
	public:
		int botAPIversion();
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
		std::string getDesc();
};

int PingReply::botAPIversion() {
	return 1002;
}

void PingReply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, nick, "PING", "");
	}
}

void PingReply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING") {
		if (message.size() > 4)
			sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
		else
			sendCTCPReply(server, nick, "PING", "");
	}
}

std::string PingReply::getDesc() {
	return "Responds to CTCP PING requests.";
}

extern "C" Module* spawn() {
	return new PingReply;
}