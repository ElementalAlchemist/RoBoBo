#include "modinclude.h"
#include "bot_admin.h"

class RawCommand : public AdminHook {
	public:
		RawCommand(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onModuleChange();
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

RawCommand::RawCommand(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : AdminHook(modConf, modFace, modName, dir, debug) {}

int RawCommand::botAPIversion() {
	return 2000;
}

bool RawCommand::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl; // debug level 1
		unloadModule();
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
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl; // debug level 1
		unloadModule();
	}
}

std::string RawCommand::description() {
	std::string desc = "Allows ";
	desc += config["masteronly"] == "yes" ? "the bot master" : "bot admins";
	desc += "to send raw IRC text.";
	return desc;
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
	rawCommand.push_back("I am not responsible for any breakage you may cause!  Usage of the raw command may desync the bot!");
	if (config["masteronly"] == "yes")
		rawCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(rawCommand);
	return theCommands;
}

void RawCommand::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "You must be the bot master to use this command.");
		else
			dccMod->dccSend(server + "/" + nick, "You must be the bot master to use this command.");
		return;
	}
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "You did not give any parameters for raw.");
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
		sendPrivMsg(server, client, nick, "Command sent.");
	else
		dccMod->dccSend(server + "/" + nick, "Command sent.");
}

MODULE_SPAWN(RawCommand)