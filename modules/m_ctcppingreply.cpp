#include "modinclude.h"

class m_ctcppingreply : public Module {
	public:
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
};

void m_ctcppingreply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING")
		sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
}

void m_ctcppingreply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "PING")
		sendCTCPReply(server, nick, "PING", message.substr(message.find_first_of(' ')));
}

extern "C" Module* spawn() {
	return new m_ctcppingreply;
}