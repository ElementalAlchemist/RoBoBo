#include "modinclude.h"
#include "bot_admin.h"

class RehashCommand : public AdminHook {
	public:
		RehashCommand(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onModuleChange();
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

RehashCommand::RehashCommand(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : AdminHook(modConf, modFace, modName, dir, debug) {}

int RehashCommand::botAPIversion() {
	return 2000;
}

bool RehashCommand::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl; // debug level 1
		unloadModule();
		return false;
	}
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
	return true;
}

void RehashCommand::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

void RehashCommand::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl;
		unloadModule();
	}
}

std::string RehashCommand::description() {
	return "Provides a way to rehash the bot configuration.";
}

std::vector<std::string> RehashCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > RehashCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> aCommand;
	aCommand.push_back("rehash");
	aCommand.push_back("Rehashes the bot configuration.");
	aCommand.push_back("Syntax: rehash");
	aCommand.push_back("This command requires no parameters.");
	aCommand.push_back("This command causes the bot to reread the configuration file and pass the changes on to the modules.");
	if (config["masteronly"] == "yes")
		aCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(aCommand);
	return theCommands;
}

void RehashCommand::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "This command is available only to the bot master.");
		else
			dccMod->dccSend(server + "/" + nick, "This command is available only to the bot master.");
		return;
	}
	rehashBot();
	if (dccMod == NULL)
		sendPrivMsg(server, client, nick, "Bot rehashed.");
	else
		dccMod->dccSend(server + "/" + nick, "Bot rehashed.");
}

MODULE_SPAWN(RehashCommand)