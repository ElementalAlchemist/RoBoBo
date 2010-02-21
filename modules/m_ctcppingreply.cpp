#include "modinclude.h"

class PingReply : public Module {
	public:
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
		std::string getDesc();
};

void PingReply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING")
		sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
}

void PingReply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING")
		sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
}

std::string PingReply::getDesc() {
	return "Responds to CTCP PING requests.";
}

extern "C" Module* spawn() {
	return new PingReply;
}