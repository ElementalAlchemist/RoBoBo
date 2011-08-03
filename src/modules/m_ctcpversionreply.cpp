#include "modinclude.h"

class VersionReply : public Module {
	public:
		VersionReply(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserCTCP(std::string server, std::string client, std::string nick, std::string message);
		std::string description();
	private:
		void sendVersionReply(std::string server, std::string client, std::string target);
};

VersionReply::VersionReply(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : Module(modConf, modFace, modName, dir, debug) {}

int VersionReply::botAPIversion() {
	return 2000;
}

bool VersionReply::onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "VERSION")
		sendVersionReply(server, client, nick);
	return true;
}

bool VersionReply::onUserCTCP(std::string server, std::string client, std::string nick, std::string message) {
	if (splitBySpace(message)[0] == "VERSION")
		sendVersionReply(server, client, nick);
	return true;
}

void VersionReply::sendVersionReply(std::string server, std::string client, std::string target) {
	std::string versionReply = "RoBoBo-IRC-BoBo IRC Bot ";
	unsigned int thisBotVersion = botVersion();
	switch (thisBotVersion) { // get bot version, so this mod isn't restricted to one version of the bot and doesn't have to be updated with each version
		case 900:
			versionReply += "v1.0.0a1";
			break;
		case 910:
			versionReply += "v1.0.0a2";
			break;
		case 950:
			versionReply += "v1.0.0b1";
			break;
		case 980:
			versionReply += "v1.0.0rc1";
			break;
		case 1089:
			versionReply += "v1.1.0-devel";
			break;
		case 1090:
			versionReply += "v1.1.0a1";
			break;
		case 1093:
			versionReply += "v1.1.0b1";
			break;
		case 1096:
			versionReply += "v1.1.0rc1";
			break;
		case 1900:
			versionReply += "v2.0.0-devel";
			break;
		case 1910:
			versionReply += "v2.0.0a1";
			break;
		case 1911:
			versionReply += "v2.0.0a2";
			break;
		default:
			std::ostringstream thisVersion;
			thisVersion << "v";
			int majorVersion = thisBotVersion / 1000;
			thisVersion << majorVersion;
			thisBotVersion -= majorVersion * 1000;
			int middleVersion = thisBotVersion / 100;
			thisVersion << "." << middleVersion;
			thisBotVersion -= middleVersion * 100;
			thisVersion << "." << thisBotVersion;
			versionReply += thisVersion.str();
	} // More of these may be added as they exist.
	if (config["replystring"] != "")
		versionReply += " " + config["replystring"];
	sendCTCPReply(server, client, target, "VERSION", versionReply);
}

std::string VersionReply::description() {
	return "This module sends a reply to CTCP version.";
}

MODULE_SPAWN(VersionReply)