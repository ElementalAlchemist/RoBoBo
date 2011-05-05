#include "modinclude.h"
#include "bot_admin.h"

class DieCommand : public AdminHook {
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

int DieCommand::botAPIversion() {
	return 2000;
}

bool DieCommand::onLoadComplete() {
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

void DieCommand::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

void DieCommand::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	std::multimap<std::string, std::string>::iterator botAdminAbility = moduleAbilities.find("BOT_ADMIN");
	if (botAdminAbility == moduleAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading" << moduleName << "..." << std::endl; // debug level 1
		unloadModule(moduleName);
	}
}

std::string DieCommand::description() {
	return "Allows " + (config["masteronly"] == "yes" ? "the bot master" : "bot admins") + " to shut the bot down from IRC.";
}

std::vector<std::string> DieCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > DieCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> dieCommand;
	dieCommand.push_back("die");
	dieCommand.push_back("Makes the bot shut down.");
	dieCommand.push_back("Syntax: die [reason]");
	dieCommand.push_back("This command causes the bot to shut down.  If a reason is given, it will be used as the quit reason on all servers.");
	if (config["masteronly"] == "yes")
		dieCommand.push_back("This command is available only to bot admins.");
	theCommands.push_back(dieCommand);
	return theCommands;
}

void DieCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "The die command is available only to bot masters.");
		else
			dccMod->dccSend(server + "/" + nick, "The die command is available only to bot masters.");
		return;
	}
	std::list<std::string> connectedServers = servers();
	for (std::list<std::string>::iterator servIter = connectedServers.begin(); servIter != connectedServers.end(); ++servIter)
		quitServer(*servIter, message);
	if (dccMod != NULL) {
		std::vector<std::string> connectedDCC = dccMod->getConnections();
		for (unsigned int i = 0; i < connectedDCC.size(); i++)
			dccMod->closeDCCConnection(connectedDCC[i]);
	}
	std::cout << "Shutting down by admin command: die" << std::endl; // debug level 1
	sleep(1); // give the send queue a little time to flush quits
	std::exit(0);
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, unsigned short debug) {
	return new DieCommand (modConf, modFace, modName, debug);
}