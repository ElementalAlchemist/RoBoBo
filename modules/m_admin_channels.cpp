#include "modinclude.h"
#include "bot_admin.h"

class AdminChannelControl : public AdminHook {
	public:
		int botAPIversion();
		void onLoadComplete();
		void onRehash();
		std::string getDesc();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

int AdminChannelControl::botAPIversion() {
	return 1000;
}

void AdminChannelControl::onLoadComplete() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator ableIter = modAbilities.find("BOT_ADMIN");
	if (ableIter == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

void AdminChannelControl::onRehash() {
	std::multimap<std::string, std::string> modAbilities = getModAbilities();
	std::multimap<std::string, std::string>::iterator ableIter = modAbilities.find("BOT_ADMIN");
	if (ableIter == modAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading..." << std::endl;
		unloadModule(moduleName);
	}
}

std::string AdminChannelControl::getDesc() {
	return "Allows admins to make the bot join and part channels.";
}

std::vector<std::string> AdminChannelControl::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > AdminChannelControl::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> joinCommand;
	joinCommand.push_back("join");
	joinCommand.push_back("Makes the bot join a channel.");
	joinCommand.push_back("Syntax: join <#channel> [key]");
	joinCommand.push_back("Syntax: join <server>/<#channel> [key]");
	joinCommand.push_back("Example: join #robobo");
	joinCommand.push_back("Example: join #privatechan thepassword");
	joinCommand.push_back("Example: join irc.server.net/#channel");
	joinCommand.push_back("This command makes RoBoBo join a channel.  The server part of the command is only necessary if you want RoBoBo to join a channel on a server different from the one from which you identified.");
	theCommands.push_back(joinCommand);
	std::vector<std::string> partCommand;
	partCommand.push_back("part");
	partCommand.push_back("Makes the bot part a channel.");
	partCommand.push_back("Syntax: part <#channel> <reason>");
	partCommand.push_back("Syntax: part <server>/<#channel> <reason>");
	partCommand.push_back("Example: part #robobo Bye.");
	partCommand.push_back("Example: part irc.server.net/#channel It's only when you look at ants through a magnifying glass that you realize how often they burst into flames.");
	partCommand.push_back("This command makes RoBoBo part a channel.  The server part of the command is only necessary if you want RoBoBo to part a channel on a server different from the one from which you identified.");
	theCommands.push_back(partCommand);
	return theCommands;
}

void AdminChannelControl::onAdminCommand(std::string server, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	std::string msgServer = message.substr(0, message.find_first_of('/'));
	if (msgServer != message) {
		server = msgServer;
		message = message.substr(message.find_first_of('/') + 1);
	}
	std::string channel = message.substr(0, message.find_first_of(' '));
	std::string keason = "";
	if (channel != message) {
		message = message.substr(0, message.find_first_of(' ') + 1);
		keason = message;
	}
	if (command == "join")
		joinChannel(server, channel, keason);
	else if (command == "part")
		partChannel(server, channel, keason);
}

extern "C" Module* spawn() {
	return new AdminChannelControl;
}