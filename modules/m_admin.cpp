#include "modinclude.h"
#include "bot_admin.h"

class Admin : public AdminMod {
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
		std::vector<std::string> supports();
		void sendVerbose(int verboseLevel, std::string message);
	private:
		std::vector<std::string> identified;
		std::vector<std::tr1::unordered_map<std::string, std::string> > admins;
		std::vector<int> verbosity;
		std::vector<bool> loggedIn;
		dccSender* dccMod;
		bool isValidVerboseLevel(std::string verboseLevel);
		void handleDCCMessage(std::string server, std::string nick, std::string message);
		std::tr1::unordered_map<std::string, std::vector<std::string> > botAdminCommands;
};

void Admin::onLoadComplete() {
	std::multimap<std::string, std::string> services = getModAbilities();
	std::multimap<std::string, std::string>::iterator serviceIter = services.find("DCC_CHAT");
	if (serviceIter == services.end())
		dccMod = NULL;
	else {
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = getModules().find(serviceIter->second);
		dccMod = (dccSender*) modIter->second;
		if (!dccMod->hookDCCMessage(moduleName, "login")) { // hook DCC message
			if (!dccMod->hookDCCMessage(moduleName, "admin")) { // backup
				dccMod = NULL; // couldn't hook messages, so no DCC
			}
		}
	}
	
	std::tr1::unordered_map<std::string, std::string> adminPrivs;
	int i = 0;
	while (true) {
		std::ostringstream adminIndex;
		adminIndex << i;
		if (config[adminIndex.str()+"/nick"] == "")
			break;
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[adminIndex.str()+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[adminIndex.str()+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[adminIndex.str()+"/password"]));
		if (isValidVerboseLevel(config[adminIndex.str()+"/verbose"]))
			adminPrivs.insert(std::pair<std::string, std::string> ("verbose", config[adminIndex.str()+"/verbose"]));
		else {
			std::cout << "Unloading m_admin: invalid configuration.  Check your verbose levels." << std::endl;
			unloadModule(moduleName);
		}
		admins.push_back(adminPrivs);
		verbosity.push_back(0); // verbosity should only be >0 with an open DCC chat session
		loggedIn.push_back(false);
		i++;
	}
	
	std::tr1::unordered_map<std::string, Module*> loadedModules = getModules();
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = loadedModules.begin(); modIter != loadedModules.end(); ++modIter) {
		std::vector<std::string> modSupports = modIter->second->supports();
		for (unsigned int i = 0; i < modSupports.size(); i++) {
			if (modSupports[i] == "BOT_ADMIN") {
				AdminHook* adminCommandMod = (AdminHook*) modIter->second;
				std::vector<std::vector<std::string> > adminSupport = adminCommandMod->adminCommands();
				for (unsigned int i = 0; i < adminSupport.size(); i++) {
					std::string command = adminSupport[i][0];
					adminSupport[i][0] = modIter->first;
					botAdminCommands.insert(std::pair<std::string, std::vector<std::string> > (command, adminSupport[i]));
				}
				break;
			}
		}
	}
}

void Admin::onRehash() {
	admins.clear();
	std::tr1::unordered_map<std::string, std::string> adminPrivs;
	int i = 0;
	while (true) {
		std::ostringstream adminIndex;
		adminIndex << i;
		if (config[adminIndex.str()+"/nick"] == "")
			break;
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[adminIndex.str()+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[adminIndex.str()+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[adminIndex.str()+"/password"]));
		if (isValidVerboseLevel(config[adminIndex.str()+"/verbose"]))
			adminPrivs.insert(std::pair<std::string, std::string> ("verbose", config[adminIndex.str()+"/verbose"]));
		else {
			std::cout << "Unloading m_admin: invalid configuration.  Check your verbose levels." << std::endl;
			unloadModule(moduleName);
		}
		admins.push_back(adminPrivs);
		verbosity.push_back(0); // verbosity should only be >0 with an open DCC chat session
		loggedIn.push_back(false);
		i++;
	}
	
	botAdminCommands.clear();
	std::tr1::unordered_map<std::string, Module*> loadedModules = getModules();
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = loadedModules.begin(); modIter != loadedModules.end(); ++modIter) {
		std::vector<std::string> modSupports = modIter->second->supports();
		for (unsigned int i = 0; i < modSupports.size(); i++) {
			if (modSupports[i] == "BOT_ADMIN") {
				AdminHook* adminCommandMod = (AdminHook*) modIter->second;
				std::vector<std::vector<std::string> > adminSupport = adminCommandMod->adminCommands();
				for (unsigned int i = 0; i < adminSupport.size(); i++) {
					std::string command = adminSupport[i][0];
					adminSupport[i][0] = modIter->first;
					botAdminCommands.insert(std::pair<std::string, std::vector<std::string> > (command, adminSupport[i]));
				}
				break;
			}
		}
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
	if (dccMod == NULL) {
		for (unsigned int i = 0; i < loggedIn.size(); i++) {
			if ((loggedIn[i] && admins[i]["server"] == server && admins[i]["nick"] == nick)) {
				handleDCCMessage(server, nick, message);
				dccMsg = true;
				break;
			}
			if (admins[i]["server"] == server && admins[i]["nick"] == nick && splitBySpace(message)[0] == "login") {
				if (message.size() <= 6)
					sendPrivMsg(server, nick, "Usage: login <password>");
				else {
					if (admins[i]["password"] == splitBySpace(message)[1]) {
						loggedIn[i] = true;
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
	for (unsigned int i = 0; i < admins.size(); i++) {
		if (loggedIn[i] && verbosity[i] == 0) { // DCC chat can persist after QUIT. For our purposes a verbosity of 0 is a query (though not necessarily)
			if (admins[i]["server"] == server && admins[i]["nick"] == hostmask.substr(0, hostmask.find_first_of('!')))
				loggedIn[i] = false;
		}
	}
	// possibly do something here, depending on verbosity levels
}

void Admin::onNickChange(std::string server, std::string oldNick, std::string newNick) {
	for (unsigned int i = 0; i < admins.size(); i++) {
		if (loggedIn[i]) {
			if (admins[i]["server"] == server && admins[i]["nick"] == oldNick)
				admins[i]["nick"] = newNick;
		}
	}
	// possibly do something depending on verbosity levels
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
		if (dccid == admins[i]["server"] + "/" + admins[i]["nick"]) {
			verbosity[i] = 0;
			loggedIn[i] = false;
		}
	}
}

std::string Admin::getDesc() {
	return "This module provides administration features for the bot.";
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
	if (verboseLevelNum >= 0 && verboseLevelNum <= 2) // all currently supported verbose levels
		return true;
	return false;
}

void Admin::handleDCCMessage(std::string server, std::string nick, std::string message) {
	std::vector<std::string> splitMsg = splitBySpace(message);
	if (splitMsg[0] == "login" || splitMsg[0] == "admin") {
		int adminNum = -1;
		for (unsigned int i = 0; i < admins.size(); i++) {
			if (admins[i]["server"] == server && admins[i]["nick"] == nick) {
				adminNum = (int) i;
				break;
			}
		}
		if (dccMod != NULL) { // just in case.  The login code for no DCC is handled in onUserMsg
			if (splitMsg.size() == 1) {
				dccMod->dccSend(server + "/" + nick, "Usage: " + splitMsg[0] + " <password>");
				dccMod->unhookDCCSession(moduleName, server + "/" + nick);
				return;
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
			}
			sendVerbose(1, "Admin " + nick + " has logged in.");
			dccMod->dccSend(server + "/" + nick, "You are now identified.");
		} // at this point we've returned out all failures, so do the necessary stuff on authentication
		loggedIn[adminNum] = true;
		std::istringstream adminVerbosityLevel (config[adminNum+"/verbose"]);
		int verbose;
		adminVerbosityLevel >> verbose;
		verbosity[adminNum] = verbose;
	} else if (splitMsg[0] == "modules") {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Loaded modules:");
		else
			dccMod->dccSend(server + "/" + nick, "Loaded modules:");
		std::tr1::unordered_map<std::string, Module*> loadedModules = getModules();
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = loadedModules.begin(); modIter != loadedModules.end(); ++modIter) {
			if (dccMod == NULL)
				sendPrivMsg(server, nick, modIter->first + ": " + modIter->second->getDesc());
			else
				dccMod->dccSend(server + "/" + nick, modIter->first + ": " + modIter->second->getDesc());
		}
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "End of module list.");
		else
			dccMod->dccSend(server + "/" + nick, "End of module list.");
	} else if (splitMsg[0] == "help") {
		if (splitMsg.size() == 1) {
			if (dccMod == NULL) {
				sendPrivMsg(server, nick, "RoBoBo Admin Help");
				sendPrivMsg(server, nick, "help -> Gives help on using commands.");
				sendPrivMsg(server, nick, "modules -> Lists loaded modules.");
				sendPrivMsg(server, nick, "active -> Lists logged-in bot administrators.");
				for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.begin(); comIter != botAdminCommands.end(); ++comIter)
					sendPrivMsg(server, nick, comIter->first + " -> " + comIter->second[1]);
				sendPrivMsg(server, nick, "End of command list!");
			} else {
				dccMod->dccSend(server + "/" + nick, "RoBoBo Admin Help");
				dccMod->dccSend(server + "/" + nick, "help -> Gives help on using commands.");
				dccMod->dccSend(server + "/" + nick, "modules -> Lists loaded modules.");
				dccMod->dccSend(server + "/" + nick, "active -> Lists logged-in bot administrators.");
				for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.begin(); comIter != botAdminCommands.end(); ++comIter)
					dccMod->dccSend(server + "/" + nick, comIter->first + " -> " + comIter->second[1]);
				dccMod->dccSend(server + "/" + nick, "End of command list!");
			}
		} else {
			if (splitMsg[1] == "modules") {
				if (dccMod == NULL) {
					sendPrivMsg(server, nick, "modules implemented by module " + moduleName);
					sendPrivMsg(server, nick, "Lists loaded modules.");
					sendPrivMsg(server, nick, "This command lists all loaded modules and their descriptions.  This command takes no arguments.");
					sendPrivMsg(server, nick, "End of help for modules");
				} else {
					dccMod->dccSend(server + "/" + nick, "modules implemented by module " + moduleName);
					dccMod->dccSend(server + "/" + nick, "Lists loaded modules.");
					dccMod->dccSend(server + "/" + nick, "This command lists all loaded modules and their descriptions.  This command takes no arguments.");
					dccMod->dccSend(server + "/" + nick, "End of help for modules.");
				}
				return;
			}
			if (splitMsg[1] == "help") {
				if (dccMod == NULL)
					sendPrivMsg(server, nick, "Silly you!  You're using that!");
				else
					dccMod->dccSend(server + "/" + nick, "Silly you!  You're using that!");
				return;
			}
			if (splitMsg[1] == "active") {
				if (dccMod == NULL) {
					sendPrivMsg(server, nick, "active implemented by module " + moduleName);
					sendPrivMsg(server, nick, "Lists logged-in bot administrators.");
					sendPrivMsg(server, nick, "This command lists the nicks and servers of all online administrators.  This command takes no arguments.");
					sendPrivMsg(server, nick, "End of help for active");
				} else {
					dccMod->dccSend(server + "/" + nick, "active implemented by module " + moduleName);
					dccMod->dccSend(server + "/" + nick, "Lists logged-in bot administrators.");
					dccMod->dccSend(server + "/" + nick, "This command lists the nicks and servers of all online administrators.  This command takes no arguments.");
					dccMod->dccSend(server + "/" + nick, "End of help for active");
				}
				return;
			}
			std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.find(splitMsg[1]);
			if (comIter == botAdminCommands.end()) {
				if (dccMod == NULL)
					sendPrivMsg(server, nick, "That command does not exist.");
				else
					dccMod->dccSend(server + "/" + nick, "That command does not exist.");
				return;
			}
			if (dccMod == NULL) {
				sendPrivMsg(server, nick, comIter->first + " implemented by module " + comIter->second[0]);
				sendPrivMsg(server, nick, comIter->second[1]);
				for (unsigned int i = 2; i < comIter->second.size(); i++)
					sendPrivMsg(server, nick, comIter->second[i]);
				sendPrivMsg(server, nick, "End of help for " + comIter->first);
			} else {
				dccMod->dccSend(server + "/" + nick, comIter->first + " implemented by module " + comIter->second[0]);
				dccMod->dccSend(server + "/" + nick, comIter->second[1]);
				for (unsigned int i = 2; i < comIter->second.size(); i++)
					dccMod->dccSend(server + "/" + nick, comIter->second[i]);
				dccMod->dccSend(server + "/" + nick, "End of help for " + comIter->first);
			}
		}
	} else if (splitMsg[0] == "active") {
		if (dccMod == NULL) {
			sendPrivMsg(server, nick, "Online Bot Administrators");
			for (unsigned int i = 0; i < admins.size(); i++) {
				if (loggedIn[i])
					sendPrivMsg(server, nick, admins[i]["nick"] + ", using server " + admins[i]["server"]);
			}
			sendPrivMsg(server, nick, "End of online admin list.");
		} else {
			dccMod->dccSend(server + "/" + nick, "Online Bot Administrators");
			for (unsigned int i = 0; i < admins.size(); i++) {
				if (loggedIn[i])
					dccMod->dccSend(server + "/" + nick, admins[i]["nick"] + ", using server " + admins[i]["server"]);
			}
			dccMod->dccSend(server + "/" + nick, "End of online admin list.");
		}
	} else {
		std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.find(splitMsg[0]);
		if (comIter == botAdminCommands.end()) {
			if (dccMod == NULL)
				sendPrivMsg(server, nick, "That command does not exist.");
			else
				dccMod->dccSend(server + "/" + nick, "That command does not exist.");
			return;
		}
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = getModules().find(comIter->second[0]);
		AdminHook* adminCommandModule = (AdminHook*) modIter->second;
		adminCommandModule->onAdminCommand(server, nick, splitMsg[0], message.substr(splitMsg[0].size() + 1), dccMod, (server == admins[0]["server"] && nick == admins[0]["nick"]));
	}
}

void Admin::sendVerbose(int verboseLevel, std::string message) {
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