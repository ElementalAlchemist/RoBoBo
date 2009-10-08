#include "robobo.h"
#include "channel.cpp"
#include "configuration.cpp"
#include "lineParser.cpp"
#include "server.cpp"
#include "socket.cpp"

std::string input, command, currentNick;
std::vector<std::string> inputParams;
bool registered;

int main(int argc, char** argv) {
	ConfigReader config;
	Socket bot_socket (config.getServer(), config.getPort());
	Server serverSettings;
	bot_socket.sendMsg("NICK :RoBoBo");
	bot_socket.sendMsg("USER RoBoBo here " + config.getServer() + " :RoBoBo-IRC-BoBo IRC Bot");
	currentNick = "RoBoBo";
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
	}
}