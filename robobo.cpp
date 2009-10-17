#include "robobo.h"

std::string input, command, currentNick, ident;
std::vector<std::string> inputParams;
std::tr1::unordered_map<std::string, Server> connectedServers;
bool registered;

void makeServerList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.serverConfig();
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator;
	for (serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++) {
		connectedServer[serverIterator->first] = Server(serverIterator->second);
	}
}

int main(int argc, char** argv) {
	ConfigReader config;
	//Socket bot_socket (config.getServer(), config.getPort());
	makeServerList(config);
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