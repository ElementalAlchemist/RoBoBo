#include "modinclude.h"
#include "bot_admin.h"

class LoadModCommand : public AdminHook {
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

int LoadModCommand::botAPIversion() {
	return 2000;
}

bool LoadModCommand::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) { // BOT_ADMIN not provided but required for this module
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

void LoadModCommand::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

void LoadModCommand::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	std::multimap<std::string, std::string>::iterator botAdminAbility = moduleAbilities.find("BOT_ADMIN");
	if (botAdminAbility == moduleAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading" << moduleName << "..." << std::endl; // debug level 1
		unloadModule(moduleName);
	}
}

std::string LoadModCommand::description() {
	return "Allows admins to load modules.";
}

std::vector<std::string> LoadModCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > LoadModCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> aCommand;
	aCommand.push_back("loadmod"); // command
	aCommand.push_back("Loads a module."); // description
	aCommand.push_back("Syntax: loadmod <modulename>"); // help
	aCommand.push_back("Example: loadmod m_admin.so");
	aCommand.push_back("Loads the specified module if it exists and isn't already loaded.");
	if (config["masteronly"] == "yes")
		aCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(aCommand);
	return theCommands;
}

void LoadModCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "You must be the bot master to use this command.");
		else
			dccMod->dccSend(server + "/" + nick, "You must be the bot master to use this command.");
		return;
	}
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Usage: loadmod <modulename>");
		else
			dccMod->dccSend(server + "/" + nick, "Usage: loadmod <modulename>");
		return;
	}
	if (loadModule(message)) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Module " + message + " loaded successfully.");
		else
			dccMod->dccSend(server + "/" + nick, "Module " + message + " loaded successfully.");
	} else {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "Module " + message + " failed to load.");
		else
			dccMod->dccSend(server + "/" + nick, "Module " + message + " failed to load.");
	}
}

extern "C" Module* spawn() {
	return new LoadModCommand;
}