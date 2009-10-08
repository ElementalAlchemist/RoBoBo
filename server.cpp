#include "server.h"

class Server {
	public:
		Server();
		void handleCapab(std::vector<std::string> parsedLine);
	private:
		std::vector<char> statusModes[2];
		std::vector<char> listModes, allParamModes, setParamModes, noParamModes, chanTypes;
};

Server::Server() {
	// initialize variables if necessary
}

void Server::handleCapab(std::vector<std::string> parsedLine) {
	// this will handle the 005 capab lines in the future
}