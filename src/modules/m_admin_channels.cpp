#include "modinclude.h"
#include "bot_admin.h"

class AdminChannelControl : public AdminHook {
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

int AdminChannelControl::botAPIversion() {
	return 2000;
}

bool AdminChannelControl::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but it is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl; // debug level 1
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

void AdminChannelControl::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'y')
			config["masteronly"] = "yes";
		else
			config["masteronly"] = "no";
	} else
		config["masteronly"] = "no";
}

void AdminChannelControl::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but it is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl;
		unloadModule();
	}
}

std::string AdminChannelControl::description() {
	return "Allows " + (config["masteronly"] == "yes" ? "the bot master" : "bot admins") + " to make the bot join and part channels.";
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
	if (config["masteronly"] == "yes")
		joinCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(joinCommand);
	std::vector<std::string> partCommand;
	partCommand.push_back("part");
	partCommand.push_back("Makes the bot part a channel.");
	partCommand.push_back("Syntax: part <#channel> <reason>");
	partCommand.push_back("Syntax: part <server>/<#channel> <reason>");
	partCommand.push_back("Example: part #robobo Bye.");
	partCommand.push_back("Example: part irc.server.net/#channel It's only when you look at ants through a magnifying glass that you realize how often they burst into flames.");
	partCommand.push_back("This command makes RoBoBo part a channel.  The server part of the command is only necessary if you want RoBoBo to part a channel on a server different from the one from which you identified.");
	if (config["masteronly"] == "yes")
		partCommand.push_back("This command is only available to bot masters.");
	theCommands.push_back(partCommand);
	return theCommands;
}

void AdminChannelControl::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "This command is only available to bot masters.");
		else
			dccMod->dccSend(server + "/" + nick, "This command is available only to bot masters.");
		return;
	}
	std::string msgServer = message.substr(0, message.find_first_of('/'));
	if (msgServer != message) {
		server = msgServer;
		message = message.substr(message.find_first_of('/') + 1);
	}
	std::string channel = message.substr(0, message.find_first_of(' '));
	std::string keason = "";
	if (channel != message)
		keason = message.substr(message.find_first_of(' ') + 1);
	if (command == "join")
		joinChannel(server, channel, keason);
	else if (command == "part")
		partChannel(server, channel, keason);
}

extern "C" Module* spawn(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) {
	return new AdminChannelControl (modConf, modFace, modName, dir, debug);
}