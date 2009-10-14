#include "robobo.h"
#include "channel.cpp"
#include "configuration.cpp"
#include "lineParser.cpp"
#include "server.cpp"
#include "socket.cpp"
#include "moduleBase.cpp"

std::string input, command, currentNick, ident;
std::vector<std::string> inputParams;
std::tr1::unordered_map<std::string, Channel> channels;
bool registered;

int main(int argc, char** argv) {
	ConfigReader config;
	Socket bot_socket (config.getServer(), config.getPort());
	Server serverSettings;
	currentNick = config.getNick();
	ident = config.getIdent();
	bot_socket.sendMsg("NICK :" + currentNick);
	bot_socket.sendMsg("USER " + ident + " here " + config.getServer() + " :RoBoBo-IRC-BoBo IRC Bot");
	while (true) {
		if (!bot_socket.isConnected()) {
			bot_socket.closeConnection();
			std::cout << "Disconnected from server." << std::endl;
			return 0;
		}
		input = bot_socket.receive();
		std::cout << input << std::endl;
		inputParams.clear();
		inputParams = parseLine(input);
		command = inputParams[1];
		if (command == "001")
			registered = true;
		if (command == "005")
			serverSettings.handleCapab(inputParams);
		if (command == "433" && !registered) {
			currentNick += "_";
			bot_socket.sendMsg("NICK :" + currentNick);
		}
		if (command == "JOIN") {
			std::pair<std::string, Channel> joinedChannel (inputParams[2], Channel (serverSettings));
			channels.insert(joinedChannel);
		}
		if (command == "PRIVMSG" && inputParams[3] == "join") {
			bot_socket.sendMsg("JOIN #zbwerewolf");
		}
	}
}