#include "modinclude.h"
#include "bot_admin.h"

class LoadModCommand : public AdminHook {
	public:
		void onLoadComplete();
		std::string getDesc();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

void LoadModCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator botAdminAbility = modAbilities.find("BOT_ADMIN");
	if (botAdminAbility == modAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for m_admin_loadmod.  Unloading." << std::endl;
		unloadModule(moduleName);
	}
}

std::string LoadModCommand::getDesc() {
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
	theCommands.push_back(aCommand);
	return theCommands;
}

void LoadModCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
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