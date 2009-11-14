#include "connection.h"

#ifndef SERVER_ROBOBO
#define SERVER_ROBOBO
Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars) {
	serverConf = confVars;
	std::istringstream portNumber (serverConf["port"]);
	unsigned int port;
	portNumber >> port;
	serverConnection.connectServer(serverAddress, port);
	handleData();
}

void Server::joinChannel(std::string channelName) {
	serverConnection.sendData("JOIN " + channelName);
}

// other interface functions

void Server::handleData() {
	std::string receivedLine = "";
	while (true) {
		receivedLine = serverConnection.receive();
		// eventually this will interpret the data received.
		// note: when you get that far, Channel takes this as a parameter
	}
}

void Server::parse005(std::vector<std::string> parsedLine) {
	for (unsigned int i = 3; i < parsedLine.size(); i++) {
		if (parsedLine[i] == "NAMESX")
			serverConnection.sendData("PROTOCTL NAMESX");
		// other parts will come soon
	}
}

std::vector<std::string> Server::parseLine(std::string unformattedLine) {
	std::vector<std::string> parsedLine;
	std::string linePart = "";
	for (unsigned int i = 0; i < unformattedLine.size(); i++) {
		if (unformattedLine[i] == ' ') {
			parsedLine.push_back(linePart);
			linePart = "";
			continue;
		}
		if (i > 0) {
			if (unformattedLine[i] == ':' && unformattedLine[i-1] == ' ') {
				for (i++; i < unformattedLine.size(); i++)
					linePart += unformattedLine[i];
				parsedLine.push_back(linePart);
				break;
			}
		}
		linePart += unformattedLine[i];
	}
	if (linePart != "")
		parsedLine.push_back(linePart);
	return parsedLine;
}

std::vector<std::string> Server::separateBySpace(std::string joinedLine) {
	std::vector<std::string> words;
	std::string wordPart;
	for (unsigned int i = 0; i < joinedLine.size(); i++) {
		if (joinedLine[i] == ' ') {
			words.push_back(wordPart);
			wordPart = "";
			continue;
		}
		wordPart += joinedLine[i];
	}
	if (wordPart != "")
		words.push_back(wordPart);
	return words;
}
#endif