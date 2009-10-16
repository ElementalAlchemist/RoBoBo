#include "server.h"

class Server {
	public:
		Server(std::tr1::unordered_map<std::string, std::string> serverConfig);
		void parseCapab(std::vector<std::string> line005);
		void sendMsg(std::string message);
		bool isConnected();
	private:
		std::string receiveLine();
		std::string networkName;
		std::vector<char> statusModes, chanTypes, chanModes;
		short numModes;
		Socket connection ();
};

Server::Server(std::tr1::unordered_map<std::string, std::string> serverConfig) {
	unsigned short port;
	std::istringstream portChange(serverConfig["port"]);
	portChange >> port;
	if (!portChange) {
		perror("A port was specified incorrectly in the configuration file.");
		exit(0);
	}
	connection.connect(serverConfig["address"], port);
	// handle the whole connection to the server soon
}

void Server::parseCapab(std::vector<std::string> line005) {
	// I'll handle this when I get there.
}

void Server::sendMsg(std::string message) {
	std::cout << networkName << ">" << message << std::endl;
	message += "\r\n";
	connection.send(message);
}

std::string Server::receiveLine() {
	if (connection.isConnected)
		return connection.receive();
	else
		return "";
}

bool Server::isConnected() {
	return connection.connected;
}