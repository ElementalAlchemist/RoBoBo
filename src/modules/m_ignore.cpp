#include "modinclude.h"
#include "stringpattern.h"
#include "bot_admin.h"

class Ignore : public AdminHook {
	public:
		int botAPIversion();
		Priority receivePriority();
		bool onLoadComplete();
		void onRehash();
		void onModuleChange();
		bool onChannelMsg(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserMsg(std::string server, std::string client, std::string nick, std::string message);
		bool onChannelNotice(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserNotice(std::string server, std::string client, std::string nick, std::string message);
		bool onChannelAction(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserAction(std::string server, std::string client, std::string nick, std::string message);
		bool onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserCTCP(std::string server, std::string client, std::string nick, std::string message);
		bool onChannelCTCPReply(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message);
		bool onUserCTCPReply(std::string server, std::string client, std::string nick, std::string message);
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands;
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
	private:
		std::list<std::string> blockedHostmasks;
		PatternMatcher* globMatcher;
		bool ignoreUser();
};

int Ignore::botAPIversion() {
	return 2000;
}

Priority Ignore::receivePriority() {
	return PRI_MEDIUM_HIGH; // allow critical modules to avoid blocking by ignore
}

bool Ignore::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("PATTERN_GLOB") == moduleAbilities.end()) {
		std::cout << "The ignore module requires a glob pattern matching module to be loaded.  Unloading " << moduleName << "..." << std::endl;
		unloadModule();
		return false;
	}
	globMatcher = modules().find(moduleAbilities.find("PATTERN_GLOB")->second)->second;
	unsigned int i = 0;
	std::ostringstream hostKey;
	hostKey << "hostmask/0";
	while (config[hostKey.str()] != "") {
		blockedHostmasks.push_back(config[hostKey.str()]);
		hostKey.str("");
		i++;
		hostKey << "hostmask/" << i;
	}
	return true;
}

void Ignore::onRehash() {
	blockedHostmasks.clear();
	unsigned int i = 0;
	std::ostringstream hostKey;
	hostKey << "hostmask/0";
	while (config[hostKey.str()] != "") {
		blockedHostmasks.push_back(config[hostKey.str()]);
		hostKey.str("");
		i++;
		hostKey << "hostmask/" << i;
	}
}

void Ignore::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("PATTERN_GLOB") == moduleAbilities.end()) {
		unloadModule();
		return;
	}
	globMatcher = modules().find(moduleAbilities.find("PATTERN_GLOB")->second)->second;
}

bool Ignore::onChannelMsg(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onUserMsg(std::string server, std::string client, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onChannelNotice(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onUserNotice(std::string server, std::string client, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onChannelAction(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onUserAction(std::string server, std::string client, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onChannelCTCP(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onUserCTCP(std::string server, std::string client, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onChannelCTCPReply(std::string server, std::string client, std::string channel, char target, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

bool Ignore::onUserCTCPReply(std::string server, std::string client, std::string nick, std::string message) {
	std::string hostmask = nick + "!" + userIdent(server, nick) + "@" + userHost(server, nick);
	for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
		if (globMatcher->match(hostmask, *blockIter))
			return false;
	}
	return true;
}

std::string Ignore::description() {
	return "Forces bot to ignore certain users.";
}

std::vector<std::string> Ignore::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("PATTERN_GLOB");
	return supporting;
}

std::vector<std::vector<std::string> > Ignore::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> ignoreCommand;
	ignoreCommand.push_back("ignore");
	ignoreCommand.push_back("Adds a host to the ignore list.");
	ignoreCommand.push_back("Syntax: ignore <hostmask>");
	ignoreCommand.push_back("The provided hostmask will be added to the list of blocked hosts.");
	ignoreCommand.push_back("A glob pattern may be used for the hostmask (that is, the * and ? wildcards may be used).");
	theCommands.push_back(ignoreCommand);
	std::vector<std::string> unignoreCommand;
	unignoreCommand.push_back("unignore");
	unignoreCommand.push_back("Removes a host from the ignore list.");
	unignoreCommand.push_back("Syntax: unignore <hostmask>");
	unignoreCommand.push_back("Removes a host from the bot ignore list.");
	unignoreCommand.push_back("The provided hostmask must exactly match a hostmask in the ignore list.");
	theCommands.push_back(unignoreCommand);
	std::vector<std::string> ignoreListCommand;
	ignoreListCommand.push_back("ignorelist");
	ignoreListCommand.push_back("Lists hostmasks in the ignore list.");
	ignoreListCommand.push_back("Syntax: ignorelist");
	ignoreListCommand.push_back("Lists all hostmasks in the ignore list.");
	theCommands.push_back(ignoreListCommand);
	return theCommands;
}

void Ignore::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (command == "ignore") {
		if (message == "") {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "You didn't enter a hostmask to ignore.");
			else
				dccMod->dccSend(server + "/" + nick, "You didn't enter a hostmask to ignore.");
			return;
		}
		if (message.find_first_of(' ') != std::string::npos)
			message = message.substr(0, message.find_first_of(' '));
		blockedHostmasks.push_back(message);
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "Added hostmask " + message + " to ignore list.");
		else
			dccMod->dccSend(server + "/" + nick, "Added hostmask " + message + " to ignore list.");
		return;
	}
	if (command == "unignore") {
		if (message == "") {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "You didn't enter a hostmask to unignore.");
			else
				dccMod->dccSend(server + "/" + nick, "You didn't enter a hostmask to unignore.");
			return;
		}
		if (message.find_first_of(' ') != std::string::npos)
			message = message.substr(0, message.find_first_of(' '));
		for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter) {
			if (*blockIter == message) {
				blockedHostmasks.erase(blockIter);
				if (dccMod == NULL)
					sendPrivMsg(server, client, nick, "Hostmask " + message + " is no longer ignored.");
				else
					dccMod->dccSend(server + "/" + nick, "Hostmask " + message + " is no longer ignored.");
				return;
			}
		}
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "Hostmask " + message + " was not ignored.");
		else
			dccMod->dccSend(server + "/" + nick, "Hostmask " + message + " was not ignored.");
		return;
	} // command == "ignorelist"
	if (dccMod == NULL) {
		sendPrivMsg(server, client, nick, "Ignore list:");
		for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter)
			sendPrivMsg(server, client, nick, *blockIter);
		sendPrivMsg(server, client, nick, "End of ignore list.");
	} else {
		dccMod->dccSend(server + "/" + nick, "Ignore list:");
		for (std::list<std::string>::iterator blockIter = blockedHostmasks.begin(); blockIter != blockedHostmasks.end(); ++blockIter)
			dccMod->dccSend(server + "/" + nick, *blockIter);
		dccMod->dccSend(server + "/" + nick, "End of ignore list.");
	}
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {
	return new Ignore (modConf, modFace, modName, dir, debug);
}