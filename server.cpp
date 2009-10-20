#include "server.h"

#ifndef SERVER_ROBOBO
#define SERVER_ROBOBO
class Server {
	public:
		Server(std::tr1::unordered_map<std::string, std::string> serverConfig, std::tr1::unordered_map<std::string, Module> moduleList);
		void parseCapab(std::vector<std::string> line005);
		void sendMsg(std::string message);
		bool isConnected();
		std::vector<std::string> parseLine(std::string message);
		std::vector<std::string> splitBySpace(std::string unsplitLine);
		std::map<char, char> statusModes;
	private:
		void joinChannel(std::string channelName);
		std::string receiveLine();
		std::string networkName;
		std::vector<char> chanTypes;
		short maxModes;
		Socket connection ();
		std::tr1::unordered_map<std::string, Channel> channels;
		std::tr1::unordered_map<std::string, Module> modules;
};

Server::Server(std::tr1::unordered_map<std::string, std::string> serverConfig, std::tr1::unordered_map<std::string, Module>& moduleList) {
	&modules = &moduleList;
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
	for (unsigned int i = 0; i < line005.size(); i++) {
		if (line005[i] == "NAMESX")
			sendMsg("PROTOCTL NAMESX");
		if (line005[i].substr(0,5) == "MODES") {
			std::istringstream numModes(line005[i].substr(6));
			numModes >> maxModes;
		}
		if (line005[i].substr(0,7) == "NETWORK")
			networkName = line005[i].substr(8);
		if (line005[i].substr(0,6) == "PREFIX") { // qaohv)~&@%+
			std::string modeInfo = line005[i].substr(8);
			string::size_type parenPos = modeInfo.find(")");
			std::string statusChars = modeInfo.substr(0,parenPos);
			std::string statusSymbols = modeInfo.substr(parenPos+1);
			for (unsigned int i = 0; i < statusChars.size(); i++) {
				std::pair<char, char> aStatus (statusChars[i], statusSymbols[i]);
				statusModes.insert(aStatus);
			}
		}
		if (line005[i].substr(0,9) == "CHANTYPES") {
			std::string typesOfChans = line005[i].substr(10);
			for (unsigned int i = 0; i < typesOfChans.size(); i++)
				chanTypes.push_back(typesOfChans[i]);
		}
	}
}

void Server::sendMsg(std::string message) {
	message += "\r\n";
	if (connection.sendData(message))
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

std::vector<std::string> Server::splitBySpace(std::string unsplitLine) {
	std::vector<std::string> parsedLine;
	std::string word = "";
	for (unsigned int i = 0; i < unsplitLine.size(); i++) {
		if (unsplitLine[i] == ' ') {
			parsedLine.push_back(word);
			word = "";
		} else
			word += unsplitLine[i];
	}
	parsedLine.push_back(word);
	return parsedLine;
}

#endif