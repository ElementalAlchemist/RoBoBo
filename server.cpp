#include "server.h"

#ifndef SERVER_ROBOBO
#define SERVER_ROBOBO
class Server {
	public:
		Server(std::tr1::unordered_map<std::string, std::string> serverConfig);
		void parseCapab(std::vector<std::string> line005);
		void sendMsg(std::string message);
		bool isConnected();
		std::vector<std::string> parseLine(std::string message);
	private:
		void joinChannel(std::string channelName);
		std::string receiveLine();
		std::string networkName;
		std::vector<char> statusModes, chanTypes, chanModes;
		short numModes;
		Socket connection ();
		std::tr1::unordered_map<std::string, Channel> channels;
};

Server::Server(std::tr1::unordered_map<std::string, std::string> serverConfig) {
	unsigned short port;
	std::istringstream portChange(serverConfig["port"]);
	portChange >> port;
	if (!portChange) {
		perror("A port was specified incorrectly in the configuration file.");
		exit(0);
	}
	connection.connectServer(serverConfig["address"], port);
	// handle the whole connection to the server soon
}

void Server::parseCapab(std::vector<std::string> line005) {
	// I'll handle this when I get there.
}

void Server::sendMsg(std::string message) {
	message += "\r\n";
	if (connection.send(message))
		std::cout << networkName << ">" << message;
	else {
		std::cout << "Error: " << networkName << ">" << message;
		connection.closeConnection();
	}
}

std::string Server::receiveLine() {
	if (connection.isConnected())
		return connection.receive();
	else
		return "";
}

bool Server::isConnected() {
	return connection.isConnected();
}

void Server::joinChannel(std::string channelName) {
	std::pair<std::string, Channel> oneChannel(channelName, Channel());
	channels.insert(oneChannel);
}

std::vector<std::string> Server::parseLine(std::string message) {
	std::string messageString;
	std::vector<std::string> messageParams;
	messageString = "";
	for (unsigned int i = 0; i < message.size(); i++) {
		if (message[i] == ' ') {
			messageParams.push_back(messageString);
			messageString = "";
		} else if (message[i] == ':' && messageString == "" && i > 0) {
			for (i++; i < message.size(); i++)
				messageString += message[i];
			messageParams.push_back(messageString);
			messageString = "";
		} else
			messageString += message[i];
	}
	return messageParams;
}
	
#endif