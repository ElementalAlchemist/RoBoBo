#include "modinclude.h"

class VersionReply : public Module {
	public:
		int botAPIversion();
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
		std::string getDesc();
	private:
		void sendVersionReply(std::string server, std::string target);
};

int VersionReply::botAPIversion() {
	return 1002;
}

void VersionReply::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "VERSION")
		sendVersionReply(server, nick);
}

void VersionReply::onUserCTCP(std::string server, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "VERSION")
		sendVersionReply(server, nick);
}

void VersionReply::sendVersionReply(std::string server, std::string target) {
	std::string versionReply = "RoBoBo-IRC-BoBo IRC Bot ";
	switch (getBotVersion()) { // get bot version, so this mod isn't restricted to one version of the bot and doesn't have to be updated with each version
		case 900:
			versionReply += "v1.0.0a1";
			break;
		case 910:
			versionReply += "v1.0.0a2";
			break;
		case 950:
			versionReply += "v1.0.0b1";
			break;
		case 1000:
			versionReply += "v1.0.0";
			break;
	} // More of these may be added as they exist.
	if (config["replystring"] != "")
		versionReply += " " + config["replystring"];
	sendCTCPReply(server, target, "VERSION", versionReply);
}

std::string VersionReply::getDesc() {
	return "This module sends a reply to CTCP version.";
}

extern "C" Module* spawn() {
	return new VersionReply;
}