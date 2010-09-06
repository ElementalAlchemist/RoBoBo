#include "modinclude.h"
#include "bot_admin.h"

class RawCommand : public AdminHook {
	public:
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onModuleChange();
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

int RawCommand::botAPIversion() {
	return 1100;
}

bool RawCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl; // debug level 1
		unloadModule(moduleName);
		return false;
	}
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'y')
			config["masteronly"] = "yes";
		else
			config["masteronly"] = "no";
	} else
		config["masteronly"] = "no";
	return true;
}

void RawCommand::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'y')
			config["masteronly"] = "yes";
		else
			config["masteronly"] = "no";
	} else
		config["masteronly"] = "no";
}

void RawCommand::onModuleChange() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl; // debug level 1
		unloadModule(moduleName);
	}
}

std::string RawCommand::description() {
	return "Allows bot admins to send raw IRC text.";
}

std::vector<std::string> RawCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > RawCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> rawCommand;
	rawCommand.push_back("raw");
	rawCommand.push_back("Allows bot admins to send raw IRC text.");
	rawCommand.push_back("Syntax: raw <text>");
	rawCommand.push_back("Syntax: raw irc.server.net/<text>");
	rawCommand.push_back("Example: raw irc.server.net/MODE #channel +m");
	rawCommand.push_back("Allows the use of raw IRC text.  Use of this command requires knowledge of the IRC protocol.");
	rawCommand.push_back("You may optionally provide a server command to send the raw command to a server other than the one on which you authenticated to this bot.");
	rawCommand.push_back("I am not responsible for any breakage you may cause!");
	if (config["masteronly"] == "yes")
		rawCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(rawCommand);
	return theCommands;
}

void RawCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "You must be the bot master to use this command.");
		else
			dccMod->dccSend(server + "/" + nick, "You must be the bot master to use this command.");
		return;
	}
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "You did not give any parameters for raw.");
		else
			dccMod->dccSend(server + "/" + nick, "You did not give any parameters for raw.");
		return;
	}
	std::string toServer = server;
	if (message.substr(0, message.find_first_of('/')).size() < message.substr(0, message.find_first_of(' ')).size()) { // server specified
		toServer = message.substr(0, message.find_first_of('/'));
		message = message.substr(message.find_first_of('/') + 1);
	}
	sendOtherCommand(toServer, message.substr(0, message.find_first_of(' ')), message.find_first_of(' ') == std::string::npos ? "" : message.substr(message.find_first_of(' ') + 1));
	if (dccMod == NULL)
		sendPrivMsg(server, nick, "Command sent.");
	else
		dccMod->dccSend(server + "/" + nick, "Command sent.");
}

extern "C" Module* spawn() {
	return new RawCommand;
}