#include "connection.h"
#include "channel.cpp"

#ifndef SERVER_ROBOBO
#define SERVER_ROBOBO
Server::Server(std::string serverAddress, std::tr1::unordered_map<std::string, std::string> confVars, ModuleInterface* modFace) {
	serverName = serverAddress;
	serverConf = confVars;
	moduleData = modFace;
	std::istringstream portNumber (serverConf["port"]);
	unsigned int port;
	portNumber >> port;
	serverConnection.connectServer(serverAddress, port);
	serverConnection.sendData("NICK " + serverConf["nick"]);
	serverConnection.sendData("USER " + serverConf["ident"] + " here " + serverAddress + " :" + serverConf["gecos"]);
	handleData();
}

void Server::sendLine(std::string line) {
	serverConnection.sendData(line);
}

std::tr1::unordered_map<char, char> Server::getPrefixes() {
	return prefix;
}

std::vector<std::vector<char> > Server::getChanModes() {
	return chanModes;
}

std::vector<char> getChanTypes() {
	return chanTypes;
}

void Server::resyncChannels() {
	for (std::tr1::unordered_map<std::string, Channel>::iterator iter = inChannels.begin(); iter != inChannels.end(); iter++)
		serverConnection.sendData("NAMES " + iter->first);
}

void Server::handleData() {
	std::string receivedLine = "";
	std::vector<std::string> parsedLine;
	while (true) {
		receivedLine = serverConnection.receive();
		std::cout << receivedLine << std::endl;
		parsedLine = parseLine(receivedLine);
		if (parsedLine[1] == "001") { // welcome to the network
			if (serverConf["channels"] != "")
				serverConnection.sendData("JOIN " + serverConf["channels"]);
		} else if (parsedLine[1] == "005") // server features
			parse005(parsedLine);
		else if (parsedLine[1] == "332") { // channel topic
			for (std::tr1::unordered_map<std::string, Channel>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[3])
					it->second.setTopic(parsedLine[4]);
			}
		} else if (parsedLine[1] == "353") { // NAMES reply
			for (std::tr1::unordered_map<std::string, Channel>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[4])
					it->second.parseNames(separateBySpace(parsedLine[5]));
			}
		} else if (parsedLine[1] == "366") { // end of NAMES reply
			for (std::tr1::unordered_map<std::string, Channel>::iterator it = inChannels.begin(); it != inChannels.end(); it++) {
				if (it->first == parsedLine[3])
					it->second.numeric366();
			}
		} else if (parsedLine[1] == "JOIN" && serverConf["nick"] == separateNickFromFullHostmask(parsedLine[0].substr(1))) // bot joined a channel
			inChannels.insert(std::pair<std::string, Channel> (parsedLine[2], Channel (this)));
		else if (parsedLine[0] == "PING") // server ping
			serverConnection.sendData("PONG " + parsedLine[1]);
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

std::string Server::separateNickFromFullHostmask(std::string hostmask) {
	return hostmask.substr(0, hostmask.find_first_of('!'));
}
#endif