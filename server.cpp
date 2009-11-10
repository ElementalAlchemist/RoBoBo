#include "server.h"

#ifndef SERVER_ROBOBO
#define SERVER_ROBOBO
class Server {
	public:
		Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars);
		void joinChannel(std::string channelName);
		// more interface functions soon
	private:
		Socket serverConnection;
		std::tr1::unordered_map<std::string, std::string> serverConf;
		std::tr1::unordered_map<std::string, Channel> inChannels;
		std::string network;
		std::tr1::unordered_map<char, char> prefix;
		std::vector<char> chanTypes, statusMsg;
		std::vector<std::vector<char> > chanModes;
		void handleData();
		void parse005(std::vector<std::string> parsedLine);
		std::vector<std::string> parseLine(std::string unformattedLine);
		std::vector<std::string> separateBySpace(std::string joinedLine);
};

Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars) {
	serverConf = confVars;
	std::istringstream portNumber (serverConf["port"]);
	unsigned int port;
	portNumber >> port;
	serverConnection.connectServer(serverAddress, port);
	handleData();
}

void joinChannel(std::string channelName) {
	serverConnection.sendData("JOIN " + channelName);
}

void Server::handleData() {
	std::string receivedLine = "";
	while (true) {
		receivedLine = serverConnection.receive();
	}
}

void Server::parse005(std::vector<std::string> parsedLine) {
	for (unsigned int i = 3; i < parsedLine.size(); i++) {
		if (parsedLine[i] == "NAMESX")
			serverConnection.sendData("PROTOCTL NAMESX");
		// other parts will come soon
	}
}
#endif