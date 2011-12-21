#include "modinclude.h"
#include "bot_admin.h"

class AdminSay : public AdminHook {
	public:
		AdminSay(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug);
		int botAPIversion();
		bool onLoadComplete();
		void onRehash();
		void onModuleChange();
		std::string description();
		std::vector<std::string> supports();
		std::vector<std::vector<std::string> > adminCommands();
		void onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master);
};

AdminSay::AdminSay(std::tr1::unordered_map<std::string, std::string> modConf, Base* modFace, std::string modName, std::string dir, unsigned short debug) : AdminHook(modConf, modFace, modName, dir, debug) {}

int AdminSay::botAPIversion() {
	return 2001;
}

bool AdminSay::onLoadComplete() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl; // debug level 1
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

void AdminSay::onRehash() {
	if (config["masteronly"] != "") {
		if (config["masteronly"][0] == 'y')
			config["masteronly"] = "yes";
		else
			config["masteronly"] = "no";
	} else
		config["masteronly"] = "no";
}

void AdminSay::onModuleChange() {
	std::multimap<std::string, std::string> moduleAbilities = modAbilities();
	if (moduleAbilities.find("BOT_ADMIN") == moduleAbilities.end()) {
		std::cout << "A module providing BOT_ADMIN was not found, but is required for " << moduleName << ".  Unloading " << moduleName << "..." << std::endl; // debug level 1
		unloadModule();
	}
}

std::string AdminSay::description() {
	return "Allows the bot to send messages and actions to users and channels.";
}

std::vector<std::string> AdminSay::supports() {
	std::vector<std::string> supporting;
	supporting.push_back("BOT_ADMIN");
	return supporting;
}

std::vector<std::vector<std::string> > AdminSay::adminCommands() {
	std::vector<std::vector<std::string> > theCommands;
	std::vector<std::string> sayCommand;
	sayCommand.push_back("say");
	sayCommand.push_back("Sends a message to a user or channel.");
	sayCommand.push_back("Syntax: say <target> <message>");
	sayCommand.push_back("Example: say #channel Hello, everyone!");
	sayCommand.push_back("Example: say User Hi!");
	sayCommand.push_back("Example: say irc.othernetwork.net/#channel Hello, everyone!");
	sayCommand.push_back("Example: say irc.othernetwork.net/User Hi!");
	sayCommand.push_back("Example: say irc.othernetwork.net/OtherRoBoBo/#channel I'm saying things!");
	sayCommand.push_back("This command makes the bot send the specified message to the specified target.");
	sayCommand.push_back("The target can be a user or a channel.");
	sayCommand.push_back("The server portion of the target is required only when you are sending the message to a server");
	sayCommand.push_back("other than the one from which you initiated the DCC connection to RoBoBo.");
	if (config["masteronly"] == "yes")
		sayCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(sayCommand);
	std::vector<std::string> actCommand;
	actCommand.push_back("act");
	actCommand.push_back("Sends an action to a user or channel.");
	actCommand.push_back("Syntax: act <target> <action>");
	actCommand.push_back("Example: act #channel stabs User");
	actCommand.push_back("Example: act User stabs you");
	actCommand.push_back("Example: act irc.othernetwork.net/#channel is on another network!");
	actCommand.push_back("Example: act irc.othernetwork.net/OtherRoBoBo/#channel says things.");
	actCommand.push_back("Example: act irc.othernetwork.net/User greets you from another server.");
	actCommand.push_back("This command makes the bot send the specified action to the specified target.");
	actCommand.push_back("The target can be a user or a channel.");
	actCommand.push_back("The server portion of the target is required only when you are sending the message to a server");
	actCommand.push_back("other than the one from which you initiated the DCC connection to RoBoBo.");
	if (config["masteronly"] == "yes")
		actCommand.push_back("This command is available only to bot masters.");
	theCommands.push_back(actCommand);
	return theCommands;
}

void AdminSay::onAdminCommand(std::string server, std::string client, std::string nick, std::string command, std::string message, dccSender* dccMod, bool master) {
	if (config["masteronly"] == "yes" && !master) {
		if (dccMod == NULL)
			sendPrivMsg(server, client, nick, "Only the bot admin has access to this command.");
		else
			dccMod->dccSend(server + "/" + nick, "Only the bot admin has access to this command.");
		return;
	}
	std::string target = message.substr(0, message.find_first_of(' '));
	std::string targetServer;
	targetServer = target.substr(0, target.find_first_of('/'));
	if (targetServer == target)
		targetServer = server;
	else
		target = target.substr(target.find_first_of('/') + 1);
	if (target.find_first_of('/') != std::string::npos) {
		client = target.substr(0, target.find_first_of('/'));
		target = target.substr(target.find_first_of('/') + 1);
	}
	message = message.substr(message.find_first_of(' ') + 1);
	if (command == "say")
		sendPrivMsg(targetServer, client, target, message);
	if (command == "act")
		sendCTCP(targetServer, client, target, "ACTION", message);
}

MODULE_SPAWN(AdminSay)