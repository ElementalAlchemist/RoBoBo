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
		void onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

int LoadModCommand::botAPIversion() {
	return 2000;
}

bool LoadModCommand::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading" << moduleName << "..." << std::endl; // debug level 1
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
		unloadModule();
	}
}

std::string LoadModCommand::description() {
	return "Allows " + (config["masteronly"] == "yes" ? "the bot master" : "bot admins") + " to load and unload modules.";
}

std::vector<std::string> LoadModCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > LoadModCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> loadCommand;
	loadCommand.push_back("loadmod"); // command
	loadCommand.push_back("Loads a module."); // description
	loadCommand.push_back("Syntax: loadmod <modulename>"); // help
	loadCommand.push_back("Example: loadmod m_admin.so");
	loadCommand.push_back("Loads the specified module if it exists and isn't already loaded.");
	if (config["masteronly"] == "yes")
		loadCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(loadCommand);
	std::vector<std::string> unloadCommand;
	unloadCommand.push_back("unloadmod");
	unloadCommand.push_back("Unloads the specified module.");
	unloadCommand.push_back("Syntax: unloadmod <module>");
	unloadCommand.push_back("Example: unloadmod m_admin.so");
	unloadCommand.push_back("This module unloads the module specified in the command parameter.");
	if (config["masteronly"] == "yes")
		unloadCommand.push_back("This module is available only to bot masters.");
	theCommands.push_back(unloadCommand);
	return theCommands;
}

void LoadModCommand::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (command == "loadmod") {
		if (config["masteronly"] == "yes" && !master) {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "You must be the bot master to use this command.");
			else
				dccMod->dccSend(server + "/" + nick, "You must be the bot master to use this command.");
			return;
		}
		if (message == "") {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "Usage: loadmod <modulename>");
			else
				dccMod->dccSend(server + "/" + nick, "Usage: loadmod <modulename>");
			return;
		}
		if (loadModule(message)) {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "Module " + message + " loaded successfully.");
			else
				dccMod->dccSend(server + "/" + nick, "Module " + message + " loaded successfully.");
		} else {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "Module " + message + " failed to load.");
			else
				dccMod->dccSend(server + "/" + nick, "Module " + message + " failed to load.");
		}
	} else { // command == "unloadmod"
		if (config["masteronly"] == "yes" && !master) {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "This module is available only to the bot master.");
			else
				dccMod->dccSend(server + "/" + nick, "This module is available only to the bot master.");
			return;
		}
		if (message == "") {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "Usage: unloadmod <module>");
			else
				dccMod->dccSend(server + "/" + nick, "Usage: unloadmod <module>");
			return;
		}
		std::tr1::unordered_map<std::string, Module*> loadedModules = modules();
		std::tr1::unordered_map<std::string, Module*>::iterator modIter = loadedModules.find(message);
		if (modIter == loadedModules.end()) {
			if (dccMod == NULL)
				sendPrivMsg(server, client, nick, "Could not unload module " + message + ": does not exist.");
			else
				dccMod->dccSend(server + "/" + nick, "Could not unload module " + message + ": does not exist.");
			return;
		}
		unloadModule(message);
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "Unloading module " + message);
		else
			dccMod->dccSend(server + "/" + nick, "Unloading module " + message);
	}
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {
	return new LoadModCommand (modConf, modFace, modName, dir, debug);
}