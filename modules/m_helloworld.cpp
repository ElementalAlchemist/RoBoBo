#include "modinclude.h"

class m_helloworld : public Module {
	public:
		void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
		std::string getDesc();
};

void m_helloworld::onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message) {
	std::vector<std::string> splitMsg = splitBySpace(message);
	if (splitMsg[0] == "Hello")
		sendPrivMsg(server, channel, "Hello world!");
}

std::string m_helloworld::getDesc() {
	std::string helpText = "Responds to text starting with \"Hello \" with \"Hello world!\"  Intended as a simple test module.";
	return helpText;
}

extern "C" Module* spawn() {
	return new m_helloworld();
}