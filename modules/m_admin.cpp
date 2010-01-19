#include "modinclude.h"
#include "dcc_chat.h"

class Admin : public dccChat {
	public:
		void onLoadComplete();
		void onRehash();
		std::vector<std::string> getAbilities();
		void onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserMsg(std::string server, std::string nick, std::string message);
		void onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserNotice(std::string server, std::string nick, std::string message);
		void onChannelAction(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserAction(std::string server, std::string nick, std::string message);
		void onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCP(std::string server, std::string nick, std::string message);
		void onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message);
		void onUserCTCPReply(std::string server, std::string nick, std::string message);
		void onChannelJoin(std::string server, std::string channel, std::string hostmask);
		void onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason);
		void onUserQuit(std::string server, std::string hostmask, std::string reason);
		void onNickChange(std::string server, std::string oldNick, std::string newNick);
		void onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason);
		void onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param);
		void onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine);
		void onOtherData(std::string server, std::vector<std::string> parsedLine);
		void onDCCReceive(std::string dccid, std::string message);
		void onDCCEnd(std::string dccid);
		std::string getDesc();
		std::string getHelp();
		std::vector<std::string> supports();
	private:
		std::vector<std::string> identified;
		std::vector<std::tr1::unordered_map<std::string, std::string> > admins;
		std::vector<int> verbosity;
		std::vector<bool> nonDCClogin;
		bool dcc;
		bool isValidVerboseLevel(std::string verboseLevel);
		void handleDCCMessage(std::string server, std::string nick, std::string message);
		void sendVerbose(int verboseLevel, std::string message);
};

void Admin::onLoadComplete() {
	std::multimap<std::string, std::string> services = getModAbilities();
	std::multimap<std::string, std::string>::iterator serviceIter = services.find("DCC_CHAT");
	if (serviceIter == services.end())
		dcc = false;
	else {
		dcc = true;
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = getModules().find(serviceIter->second);
		dccSender* dccMod = (dccSender*) modIter->second;
		if (!dccMod->hookDCCMessage(moduleName, "login")) { // hook DCC message
			if (!dccMod->hookDCCMessage(moduleName, "admin")) { // backup
				dcc = false; // couldn't hook messages, so no DCC
			}
		}
	}
	
	std::tr1::unordered_map<std::string, std::string> adminPrivs;
	for (int i = 0; config[i+"/nick"] != ""; i++) {
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[i+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[i+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[i+"/password"]));
		if (isValidVerboseLevel(config[i+"/verbose"]))
			adminPrivs.insert(std::pair<std::string, std::string> ("verbose", config[i+"/verbose"]));
		else {
			std::cout << "Unloading m_admin: invalid configuration.  Check your verbose levels." << std::endl;
			unloadModule(moduleName);
		}
		admins.push_back(adminPrivs);
		verbosity.push_back(0); // verbosity should only be >0 with an open DCC chat session
		nonDCClogin.push_back(false); // nonDCClogin should only be true when someone logs in via PRIVMSGS
		adminPrivs.clear();
	}
}

void Admin::onRehash() {
	admins.clear();
	std::tr1::unordered_map<std::string, std::string> adminPrivs;
	for (int i = 0; config[i+"/nick"] != ""; i++) {
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[i+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[i+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[i+"/password"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("verbose", config[i+"/verbose"]));
		admins.push_back(adminPrivs);
		adminPrivs.clear();
	}
}

std::vector<std::string> Admin::getAbilities() {
	std::vector<std::string> abilities;
	abilities.push_back("BOT_ADMIN");
	return abilities;
}

void Admin::onChannelMsg(std::string server, std::string channel, char target, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserMsg(std::string server, std::string nick, std::string message) {
	bool dccMsg = false;
	if (!dcc) {
		for (unsigned int i = 0; i < nonDCClogin.size(); i++) {
			if ((nonDCClogin[i] && admins[i]["server"] == server && admins[i]["nick"] == nick)) {
				handleDCCMessage(server, nick, message);
				dccMsg = true;
				break;
			}
			if (admins[i]["server"] == server && admins[i]["nick"] == nick && splitBySpace(message)[0] == "login") {
				if (message.size() <= 6)
					sendPrivMsg(server, nick, "Usage: login <password>");
				else {
					if (admins[i]["password"] == splitBySpace(message)[1]) {
						nonDCClogin[i] = true;
						sendPrivMsg(server, nick, "You are now identified.");
					} else
						sendPrivMsg(server, nick, "You are not an admin of this bot.  Go away. :(");
				}
				dccMsg = true; // the message was handled.
				break;
			}
		}
	}
	if (!dccMsg) {
		// possibly do something here, depending on verbosity levels
	}
}

void Admin::onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserNotice(std::string server, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelAction(std::string server, std::string channel, char target, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserAction(std::string server, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserCTCP(std::string server, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserCTCPReply(std::string server, std::string nick, std::string message) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelJoin(std::string server, std::string channel, std::string hostmask) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onUserQuit(std::string server, std::string hostmask, std::string reason) {
	// possibly do something here, depending on verbosity levels
	// check nonDCClogin
}

void Admin::onNickChange(std::string server, std::string oldNick, std::string newNick) {
	// possibly do something depending on verbosity levels
	// check for nick changes to admin
}

void Admin::onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onOtherData(std::string server, std::vector<std::string> parsedLine) {
	// possibly do something here, depending on verbosity levels
}

void Admin::onDCCReceive(std::string dccid, std::string message) { // dccid = server/nick
	std::string server = dccid.substr(0, dccid.find_first_of('/'));
	std::string nick = dccid.substr(dccid.find_first_of('/')+1);
	handleDCCMessage(server, nick, message);
}

void Admin::onDCCEnd(std::string dccid) {
	for (unsigned int i = 0; i < admins.size(); i++) {
		std::tr1::unordered_map<std::string, std::string>::iterator server = admins[i].find("server");
		std::tr1::unordered_map<std::string, std::string>::iterator nick = admins[i].find("nick");
		if (dccid == server->second + "/" + nick->second)
			verbosity[i] = 0;
	}
}

std::string Admin::getDesc() {
	return "This module provides administration features for the bot.";
}

std::string Admin::getHelp() {
	return "Help coming soon.";
}

std::vector<std::string> Admin::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("DCC_CHAT");
	return supporting;
}

bool Admin::isValidVerboseLevel(std::string verboseLevel) {
	std::istringstream verboseLevelString (verboseLevel);
	int verboseLevelNum;
	verboseLevelString >> verboseLevelNum;
	if (!verboseLevelString)
		return false;
	if (verboseLevel >= 0 && verboseLevel <= 2) // all currently supported verbose levels
		return true;
	return false;
}

void Admin::handleDCCMessage(std::string server, std::string nick, std::string message) {
	std::vector<std::string> splitMsg = splitBySpace(message);
	dccSender* dccMod = getModules().find(getModAbilities().find("DCC_CHAT")->second)->second;
	if (splitMsg[0] == "login") {
		if (splitMsg.size() == 1) {
			dccMod->dccSend(server + "/" + nick, "Usage: login <password>");
			dccMod->unhookDCCSession(moduleName, server + "/" + nick);
			return;
		}
		int adminNum = -1;
		for (unsigned int i = 0; i < admins.size(); i++) {
			if (admins[i]["nick"] == nick) {
				adminNum = (int) i;
				break;
			}
		}
		if (adminNum == -1) {
			dccMod->dccSend(server + "/" + nick, "You are not an admin of this bot.  Go away.");
			dccMod->unhookDCCSession(moduleName, server + "/" + nick);
			sendVerbose(1, "Unauthorized user " + server + "/" + nick + " has attempted to authenticate with the bot.");
			return;
		}
		if (admins[adminNum]["password"] != splitMsg[1]) {
			dccMod->dccSend(server + "/" + nick, "You are not an admin of this bot.  Go away.");
			dccMod->unhookDCCSession(moduleName, server + "/" + nick);
			sendVerbose(1, "Unauthorized user " + server + "/" + nick + " has attempted to authenticate with the bot.");
			return;
		} // at this point we've returned out all failures, so do the necessary stuff on authentication
		sendVerbose(1, "Admin " + nick + " has logged in.");
		// handle logging in procedures e.g. set verbosity level etc.
	}
}

void Admin::sendVerbose(int verboseLevel, std::string message) {
	dccSender* dccMod = getModules().find(getModAbilities().find("DCC_CHAT")->second)->second;
	if (verboseLevel < 1)
		verboseLevel = 1; // verboseLevel 0 receives no messages, no matter what
	for (unsigned int i = 0; i < verbosity.size(); i++) {
		if (verbosity[i] >= verboseLevel)
			dccMod->dccSend(admins[i]["server"]+"/"+admins[i]["nick"], message);
	}
}

extern "C" Module* spawn() {
	return new Admin;
}