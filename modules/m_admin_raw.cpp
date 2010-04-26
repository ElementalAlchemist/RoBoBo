#include "modinclude.h"
#include "bot_admin.h"

class RawCommand : public AdminHook {
	public:
		void onLoadComplete();
		void onRehash();
		std::string getDesc();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

void RawCommand::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

void RawCommand::onRehash() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	if (modAbilities.find("BOT_ADMIN") == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN is required for " << moduleName << " but was not found.  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

std::string RawCommand::getDesc() {
	return "Allows bot admins to send raw IRC text.";
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
	rawCommand.push_back("I am not responsible for any breakage you may cause!");
	theCommands.push_back(rawCommand);
	return theCommands;
}

void RawCommand::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (message == "") {
		if (dccMod == NULL)
			sendPrivMsg(server, nick, "You did not give any parameters for raw.");
		else
			dccMod->dccSend(server + "/" + nick, "You did not give any parameters for raw.");
		return;
	}
	std::string toServer = server;
	if (message.substr(0, message.find_first_of('/')).size() < message.substr(0, message.find_first_of(' ')).size()) { // server specified
		toServer = message.substr(0, message.find_first_of('/'));
		message = message.substr(message.find_first_of('/') + 1);
	}
	sendOtherCommand(toServer, message.substr(0, message.find_first_of(' ')), message.substr(message.find_first_of(' ') + 1));
	if (dccMod == NULL)
		sendPrivMsg(server, nick, "Command sent.");
	else
		dccMod->dccSend(server + "/" + nick, "Command sent.");
}

extern "C" Module* spawn() {
	return new RawCommand;
}