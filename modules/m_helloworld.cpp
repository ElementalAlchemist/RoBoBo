#include "../robobo.h"

class m_helloworld : public Module {
	public:
		void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
};

void m_helloworld::onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message) {
	std::vector<std::string> splitMsg = splitBySpace(message);
	if (splitMsg[0] == "Hello")
		sendPrivMsg(server, channel, "Hello world!");
}

extern "C" Module* spawn() {
	return new m_helloworld();
}