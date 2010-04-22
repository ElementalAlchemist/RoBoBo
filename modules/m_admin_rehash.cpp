#include "modinclude.h"
#include "bot_admin.h"

class RehashCommand : public AdminHook {
	public:
		void onLoadComplete();
		void onRehash();
		std::string getDesc();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

void RehashCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator ableIter = modAbilities.find("BOT_ADMIN");
	if (ableIter == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

void RehashCommand::onRehash() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator ableIter = modAbilities.find("BOT_ADMIN");
	if (ableIter == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

std::string RehashCommand::getDesc() {
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
	aCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(aCommand);
	return theCommands;
}

void RehashCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (!master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "This command is available only to the bot master.");
		else
			dccMod->dccSend(server + "/" + nick, "This command is available only to the bot master.");
		return;
	}
	rehashBot();
	if (dccMod == NULL)
		sendPrivMsg(server, nick, "Bot rehashed.");
	else
		dccMod->dccSend(server + "/" + nick, "Bot rehashed.");
}

extern "C" Module* spawn() {
	return new RehashCommand;
}