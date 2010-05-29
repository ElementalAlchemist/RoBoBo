#include "modinclude.h"
#include "bot_admin.h"
#include <algorithm>

class Admin : public AdminMod {
	public:
		int botAPIversion();
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
		void onConnect(std::string server);
		void onQuit(std::string server);
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

int Admin::botAPIversion() {
	return 1001;
}

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
	
	int i = 0;
	while (true) {
		std::tr1::unordered_map<std::string, std::string> adminPrivs;
		std::ostringstream adminIndex;
		adminIndex << i;
		if (config[adminIndex.str()+"/server"] == "" || config[adminIndex.str()+"/nick"] == "" || config[adminIndex.str()+"/password"] == "")
			break;
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[adminIndex.str()+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[adminIndex.str()+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[adminIndex.str()+"/password"]));
		if (config[adminIndex.str()+"/verbose"] == "")
			config[adminIndex.str()+"/verbose"] = "0";
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
	verbosity.clear();
	loggedIn.clear();
	int i = 0;
	while (true) {
		std::tr1::unordered_map<std::string, std::string> adminPrivs;
		std::ostringstream adminIndex;
		adminIndex << i;
		if (config[adminIndex.str()+"/server"] == "" || config[adminIndex.str()+"/nick"] == "" || config[adminIndex.str()+"/password"] == "")
			break;
		adminPrivs.insert(std::pair<std::string, std::string> ("server", config[adminIndex.str()+"/server"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("nick", config[adminIndex.str()+"/nick"]));
		adminPrivs.insert(std::pair<std::string, std::string> ("password", config[adminIndex.str()+"/password"]));
		if (config[adminIndex.str()+"/verbose"] == "")
			config[adminIndex.str()+"/verbose"] = "0";
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
	if (target == '0')
		sendVerbose(2, server + ": " + nick + " -> " + channel + ": " + message);
	else
		sendVerbose(2, server + ": " + nick + " -> " + target + channel + ": " + message);
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
		sendVerbose(2, server + ": <" + nick + "> " + message);
	}
}

void Admin::onChannelNotice(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (target == '0')
		sendVerbose(2, server + ": -" + nick + "/" + channel + "- " + message);
	else
		sendVerbose(2, server + ": -" + nick + "/" + target + channel + "- " + message);
}

void Admin::onUserNotice(std::string server, std::string nick, std::string message) {
	sendVerbose(2, server + ": --" + nick + "-- " + message);
}

void Admin::onChannelAction(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (target == '0')
		sendVerbose(2, server + ": (" + channel + ") * " + nick + " " + message);
	else
		sendVerbose(2, server + ": (" + target + channel + ") * " + nick + " " + message);
}

void Admin::onUserAction(std::string server, std::string nick, std::string message) {
	sendVerbose(2, server + ": * " + nick + " " + message);
}

void Admin::onChannelCTCP(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (target == '0')
		sendVerbose(2, server + ": --> [" + nick + "/" + channel + "] " + message);
	else
		sendVerbose(2, server + ": --> [" + nick + "/" + target + channel + "] " + message);
}

void Admin::onUserCTCP(std::string server, std::string nick, std::string message) {
	sendVerbose(2, server + ": --> [" + nick + "] " + message);
}

void Admin::onChannelCTCPReply(std::string server, std::string channel, char target, std::string nick, std::string message) {
	if (target == '0')
		sendVerbose(2, server + ": <-- [" + nick + "/" + channel + "] " + message);
	else
		sendVerbose(2, server + ": <-- [" + nick + "/" + target + channel + "] " + message);
}

void Admin::onUserCTCPReply(std::string server, std::string nick, std::string message) {
	sendVerbose(2, server + ": <-- [" + nick + "] " + message);
}

void Admin::onChannelJoin(std::string server, std::string channel, std::string hostmask) {
	sendVerbose(2, server + ": " + hostmask + " JOIN " + channel);
}

void Admin::onChannelPart(std::string server, std::string channel, std::string hostmask, std::string reason) {
	sendVerbose(2, server + ": " + hostmask + " PART " + channel + " (" + reason + ")");
}

void Admin::onUserQuit(std::string server, std::string hostmask, std::string reason) {
	for (unsigned int i = 0; i < admins.size(); i++) {
		if (loggedIn[i] && verbosity[i] == 0) { // DCC chat can persist after QUIT. For our purposes a verbosity of 0 is a query (though not necessarily)
			if (admins[i]["server"] == server && admins[i]["nick"] == hostmask.substr(0, hostmask.find_first_of('!')))
				loggedIn[i] = false;
		}
	}
	sendVerbose(2, server + ": " + hostmask + " QUIT (" + reason + ")");
}

void Admin::onNickChange(std::string server, std::string oldNick, std::string newNick) {
	for (unsigned int i = 0; i < admins.size(); i++) {
		if (loggedIn[i]) {
			if (admins[i]["server"] == server && admins[i]["nick"] == oldNick)
				admins[i]["nick"] = newNick;
		}
	}
	sendVerbose(2, server + ": " + oldNick + " -> " + newNick);
}

void Admin::onChannelKick(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	sendVerbose(2, server + ": " + kicker + " KICK " + kickee + " from " + channel + " (" + reason + ")");
}

void Admin::onChannelMode(std::string server, std::string channel, std::string setter, char mode, bool add, std::string param) {
	sendVerbose(2, server + ": (" + channel + ") " + setter + " set MODE " + (add ? "+" : "-") + mode + " " + param);
}

void Admin::onNumeric(std::string server, std::string numeric, std::vector<std::string> parsedLine) {
	sendVerbose(2, " -> " + server + " " + numeric + " " + parsedLine[3]);
}

void Admin::onOtherData(std::string server, std::vector<std::string> parsedLine) {
	std::string message = "";
	for (unsigned int i = 0; i < parsedLine.size(); i++) {
		message += " " + parsedLine[i];
	}
	sendVerbose(2, message.substr(1));
}

void Admin::onConnect(std::string server) {
	if (!loggedIn[0])
		return;
	for (unsigned int i = 0; i < admins.size(); i++) {
		if (admins[i]["server"] == server) {
			dccMod->dccSend(admins[0]["server"] + "/" + admins[0]["nick"], "I've just connected to the server " + server + ", and there is an admin block for someone on that server.  Please rehash to update the admin list.");
			break;
		}
	}
}

void Admin::onQuit(std::string server) {
	for (std::vector<std::tr1::unordered_map<std::string, std::string> >::iterator adminIter = admins.begin(); adminIter != admins.end(); ++adminIter) {
		if ((*adminIter)["server"] == server)
			admins.erase(adminIter);
	}
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
	std::transform(splitMsg[0].begin(), splitMsg[0].end(), splitMsg[0].begin(), ::tolower);
	if (splitMsg[0] == "login" || splitMsg[0] == "admin") {
		int adminNum = -1;
		for (unsigned int i = 0; i < admins.size(); i++) {
			if (admins[i]["server"] == server && admins[i]["nick"] == nick) {
				adminNum = (int) i;
				break;
			}
		}
		if (adminNum != -1) {
			if (loggedIn[adminNum])
				return; // stop!
		}
		if (dccMod != NULL) { // the login code for no DCC is already handled in onUserMsg
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
		std::ostringstream adminIndex;
		adminIndex << adminNum;
		std::istringstream adminVerbosityLevel (config[adminIndex.str()+"/verbose"]);
		int verbose;
		adminVerbosityLevel >> verbose;
		verbosity[adminNum] = verbose;
		std::cout << "Admin " << adminNum << " logged in with verbosity level " << verbose << std::endl;
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
				sendPrivMsg(server, nick, "servers -> Lists connected servers.");
				sendPrivMsg(server, nick, "admins -> Lists logged-in bot administrators.");
				for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.begin(); comIter != botAdminCommands.end(); ++comIter)
					sendPrivMsg(server, nick, comIter->first + " -> " + comIter->second[1]);
				sendPrivMsg(server, nick, "End of command list!");
			} else {
				dccMod->dccSend(server + "/" + nick, "RoBoBo Admin Help");
				dccMod->dccSend(server + "/" + nick, "help -> Gives help on using commands.");
				dccMod->dccSend(server + "/" + nick, "modules -> Lists loaded modules.");
				dccMod->dccSend(server + "/" + nick, "servers -> Lists connected servers.");
				dccMod->dccSend(server + "/" + nick, "admins -> Lists logged-in bot administrators.");
				for (std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.begin(); comIter != botAdminCommands.end(); ++comIter)
					dccMod->dccSend(server + "/" + nick, comIter->first + " -> " + comIter->second[1]);
				dccMod->dccSend(server + "/" + nick, "End of command list!");
			}
		} else {
			std::transform(splitMsg[1].begin(), splitMsg[1].end(), splitMsg[1].begin(), ::tolower);
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
			if (splitMsg[1] == "admins") {
				if (dccMod == NULL) {
					sendPrivMsg(server, nick, "admins implemented by module " + moduleName);
					sendPrivMsg(server, nick, "Lists logged-in bot administrators.");
					sendPrivMsg(server, nick, "This command lists the nicks and servers of all online administrators.  This command takes no arguments.");
					sendPrivMsg(server, nick, "End of help for admins");
				} else {
					dccMod->dccSend(server + "/" + nick, "admins implemented by module " + moduleName);
					dccMod->dccSend(server + "/" + nick, "Lists logged-in bot administrators.");
					dccMod->dccSend(server + "/" + nick, "This command lists the nicks and servers of all online administrators.  This command takes no arguments.");
					dccMod->dccSend(server + "/" + nick, "End of help for admins");
				}
				return;
			}
			if (splitMsg[1] == "servers") {
				if (dccMod == NULL) {
					sendPrivMsg(server, nick, "servers implemented by module " + moduleName);
					sendPrivMsg(server, nick, "Lists servers to which bot is connected.");
					sendPrivMsg(server, nick, "This command lists all of the IRC servers to which this bot is connected.  This command takes no arguments.");
					sendPrivMsg(server, nick, "End of help for servers");
				} else {
					dccMod->dccSend(server + "/" + nick, "servers implemented by module " + moduleName);
					dccMod->dccSend(server + "/" + nick, "Lists servers to which bot is connected.");
					dccMod->dccSend(server + "/" + nick, "This command lists all of the IRC servers to which this bot is connected.  This command takes no arguments.");
					dccMod->dccSend(server + "/" + nick, "End of help for servers");
				}
				return;
			}
			std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.find(splitMsg[1]);
			if (comIter == botAdminCommands.end())
				return;
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
	} else if (splitMsg[0] == "admins") {
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
	} else if (splitMsg[0] == "servers") {
		std::list<std::string> serverList = getServers();
		if (dccMod == NULL) {
			sendPrivMsg(server, nick, "Connected Servers");
			for (std::list<std::string>::iterator servIter = serverList.begin(); servIter != serverList.end(); ++servIter)
				sendPrivMsg(server, nick, *servIter);
			sendPrivMsg(server, nick, "End of server list.");
		} else {
			dccMod->dccSend(server + "/" + nick, "Connected Servers");
			for (std::list<std::string>::iterator servIter = serverList.begin(); servIter != serverList.end(); ++servIter)
				dccMod->dccSend(server + "/" + nick, *servIter);
			dccMod->dccSend(server + "/" + nick, "End of server list.");
		}
	} else {
		std::tr1::unordered_map<std::string, std::vector<std::string> >::iterator comIter = botAdminCommands.find(splitMsg[0]);
		if (comIter == botAdminCommands.end())
			return;
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = getModules().find(comIter->second[0]);
		AdminHook* adminCommandModule = (AdminHook*) modIter->second;
		adminCommandModule->onAdminCommand(server, nick, splitMsg[0], (message.size() > splitMsg[0].size()) ? message.substr(splitMsg[0].size() + 1) : "", dccMod, (server == admins[0]["server"] && nick == admins[0]["nick"]));
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