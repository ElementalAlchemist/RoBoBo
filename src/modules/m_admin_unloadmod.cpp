#include "modinclude.h"
#include "bot_admin.h"

class UnloadModuleCommand : public AdminHook {
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

int UnloadModuleCommand::botAPIversion() {
	return 1100;
}

bool UnloadModuleCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading" << moduleName << "..." << std::endl; // debug level 1
		unloadModule(moduleName);
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

void UnloadModuleCommand::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

void UnloadModuleCommand::onModuleChange() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading" << moduleName << "..." << std::endl; // debug level 1
		unloadModule(moduleName);
	}
}

std::string UnloadModuleCommand::description() {
	return "Allows you to unload other modules.";
}

std::vector<std::string> UnloadModuleCommand::supports() {
	std::vector<std::string> modSupports;
	modSupports.push_back("BOT_ADMIN");
	return modSupports;
}

std::vector<std::vector<std::string> > UnloadModuleCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> aCommand;
	aCommand.push_back("unloadmod");
	aCommand.push_back("Unloads the specified module.");
	aCommand.push_back("Syntax: unloadmod <module>");
	aCommand.push_back("Example: unloadmod m_admin.so");
	aCommand.push_back("This module unloads the module specified in the command parameter.");
	if (config["masteronly"] == "yes")
		aCommand.push_back("This module is available only to bot masters.");
	theCommands.push_back(aCommand);
	return theCommands;
}

void UnloadModuleCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "This module is available only to the bot master.");
		else
			sendPrivMsg(server, nick, "This module is available only to the bot master.");
		return;
	}
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Usage: unloadmod <module>");
		else
			dccMod->dccSend(server + "/" + nick, "Usage: unloadmod <module>");
		return;
	}
	std::tr1::unordered_map<std::string, Module*> modules = getModules();
	std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(message);
	if (modIter == modules.end()) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Could not unload module " + message + ": does not exist.");
		else
			dccMod->dccSend(server + "/" + nick, "Could not unload module " + message + ": does not exist.");
		return;
	}
	unloadModule(message);
	if (dccMod == NULL)
		sendPrivMsg(server, nick, "Unloading module " + message);
	else
		dccMod->dccSend(server + "/" + nick, "Unloading module " + message);
}

extern "C" Module* spawn() {
	return new UnloadModuleCommand;
}