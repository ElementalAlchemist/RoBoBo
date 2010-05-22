#include "modinclude.h"
#include "bot_admin.h"

class ConnectServerCommand : public AdminHook {
	public:
		int botAPIversion();
		void onLoadComplete();
		void onRehash();
		std::string getDesc();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

int ConnectServerCommand::botAPIversion() {
	return 1000;
}

void ConnectServerCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator botAdminAbility = modAbilities.find("BOT_ADMIN");
	if (botAdminAbility == modAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading." << std::endl;
		unloadModule(moduleName);
	}
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

void ConnectServerCommand::onRehash() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator botAdminAbility = modAbilities.find("BOT_ADMIN");
	if (botAdminAbility == modAbilities.end()) { // BOT_ADMIN not provided but required for this module
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << ".  Unloading." << std::endl;
		unloadModule(moduleName);
	}
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'n')
			config["masteronly"] = "no";
		else
			config["masteronly"] = "yes";
	} else
		config["masteronly"] = "yes";
}

std::string ConnectServerCommand::getDesc() {
	return "Allows admins to connect to and disconnect from servers.";
}

std::vector<std::string> ConnectServerCommand::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > ConnectServerCommand::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> connectCommand;
	std::vector<std::string> quitCommand;
	std::vector<std::string> disconnectCommand;
	connectCommand.push_back("connect");
	connectCommand.push_back("Makes the bot connect to a server.");
	connectCommand.push_back("Syntax: connect <server>");
	connectCommand.push_back("When this command is issued, the bot will connect to the server specified.  The server parameter must be the server's address.");
	connectCommand.push_back("There must be connection information in the configuration file in order for the bot to connect to the server.  If it's not, put it in the configuration and rehash the bot (requires a rehash module to be loaded), then try to connect.");
	if (config["masteronly"] == "yes")
		connectCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(connectCommand);
	quitCommand.push_back("quit");
	quitCommand.push_back("Makes the bot quit a server.");
	quitCommand.push_back("Syntax: quit <server> <reason>");
	quitCommand.push_back("When this command is issued, the bot will quit the server specified.  The server parameter must be the server address that was used to connect.");
	if (config["masteronly"] == "yes")
		quitCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(quitCommand);
	disconnectCommand.push_back("disconnect");
	disconnectCommand.push_back("Makes the bot disconnect from a server.");
	disconnectCommand.push_back("Syntax: disconnect <server> <reason>");
	disconnectCommand.push_back("Does the same thing as quit.  See \"help quit\" for more information.");
	theCommands.push_back(disconnectCommand);
	return theCommands;
}

void ConnectServerCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "This command is available only to the bot master.");
		else
			dccMod->dccSend(server + "/" + nick, "This command is available only to the bot master.");
		return;
	}
	if (command == "connect") {
		if (message.substr(0, message.find_first_of(' ')) != message) {
			if (dccMod == NULL)
				sendPrivMsg(server, nick, "Too many parameters for connect.");
			else
				dccMod->dccSend(server + "/" + nick, "Too many parameters for connect.");
			return;
		}
		if (connectServer(message)) {
			if (dccMod == NULL)
				sendPrivMsg(server, nick, "Connecting to " + message + ".");
			else
				dccMod->dccSend(server + "/" + nick, "Connecting to " + message + ".");
		} else {
			if (dccMod == NULL)
				sendPrivMsg(server, nick, "Cannot connect to " + message + ".  Please put a server block for " + message + " in the configuration file and try again.");
			else
				dccMod->dccSend(server + "/" + nick, "Cannot connect to " + message + ".  Please put a server block for " + message  + " in the configuration file and try again.");
		}
		return;
	} // everything below is quit/disconnect
	std::string quitServerName = message.substr(0, message.find_first_of(' '));
	std::string quitReason = message.substr(message.find_first_of(' ') + 1);
	if (dccMod == NULL)
		sendPrivMsg(server, nick, "Quitting server " + quitServerName + " with reason " + quitReason);
	else
		dccMod->dccSend(server + "/" + nick, "Quitting server " + quitServerName + " with reason " + quitReason);
	quitServer(quitServerName, quitReason);
}

extern "C" Module* spawn() {
	return new ConnectServerCommand;
}