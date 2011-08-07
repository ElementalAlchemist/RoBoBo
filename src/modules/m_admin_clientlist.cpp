#include "modinclude.h"
#include "bot_admin.h"

class ClientList : public AdminHook {
	public:
		ClientList(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onLoadComplete();
		void onModuleChange();
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

ClientList::ClientList(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : AdminHook(modConf, modFace, modName, dir, debug) {}

int ClientList::botAPIversion() {
	return 2000;
}

bool ClientList::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for m_" << moduleName << "." << std::endl;
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

void ClientList::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for m_" << moduleName << "." << std::endl;
		unloadModule();
	}
}

std::string ClientList::description() {
	return "Lists clients connected to a server.";
}

std::vector<std::string> ClientList::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > ClientList::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> clientsCommand;
	clientsCommand.push_back("clients");
	clientsCommand.push_back("Lists clients connected to a server.");
	clientsCommand.push_back("Syntax: clients <server>");
	clientsCommand.push_back("All local clients connected to a specified server will be listed.  The specified server must match the name of a server block in the configuration.");
	theCommands.push_back(clientsCommand);
	return theCommands;
}

void ClientList::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "Only the bot master may use this command.");
		else
			dccMod->dccSend(server + "/" + nick, "Only the bot master may use this command.");
		return;
	}
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "You must specify a server.");
		else
			dccMod->dccSend(server + "/" + nick, "You must specify a server.");
		return;
	}
	std::set<std::string> clientList = clients(message);
	if (dccMod == NULL) {
		sendPrivMsg(server, client, nick, "Clients on server " + message + ":");
		for (std::set<std::string>::iterator clientIter = clientList.begin(); clientIter != clientList.end(); ++clientIter)
			sendPrivMsg(server, client, nick, *clientIter);
		sendPrivMsg(server, client, nick, "End of clients list.");
	} else {
		dccMod->dccSend(server + "/" + nick, "Clients on server " + message + ":");
		for (std::set<std::string>::iterator clientIter = clientList.begin(); clientIter != clientList.end(); ++clientIter)
			dccMod->dccSend(server + "/" + nick, *clientIter);
		dccMod->dccSend(server + "/" + nick, "End of clients list.");
	}
}

MODULE_SPAWN(ClientList)