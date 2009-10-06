#include "robobo.h"
#include "socket.cpp"

std::string input, command, server, currentNick;
unsigned short port;
std::vector<std::string> inputParams;
bool registered;

void handleCapab(std::vector<std::string> parsedLine) {
	// handle 005 when I get that far
}

int main(int argc, char** argv) {
	server = "128.237.157.136";
	port = 6667;
	Socket bot_socket (server, port);
	bot_socket.sendMsg("NICK :RoBoBo");
	bot_socket.sendMsg("USER RoBoBo here " + server + " :RoBoBo-IRC-BoBo IRC Bot");
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
		inputParams = bot_socket.parseLine(input);
		command = inputParams[1];
		std::cout << "Command/Numeric: " << command << std::endl;
		if (command == "001")
			registered = true;
		if (command == "005")
			handleCapab(inputParams);
		if (command == "433" && !registered) {
			currentNick += "_";
			bot_socket.sendMsg("NICK :" + currentNick);
		}
	}
}