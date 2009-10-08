#include "server.h"

class Server {
	public:
		Server();
		void handleCapab(std::vector<std::string> parsedLine);
		std::vector<char> prefixModes(bool symbols);
	private:
		std::vector<std::vector<char> > statusModes;
		std::vector<char> listModes, allParamModes, setParamModes, noParamModes, chanTypes;
};

Server::Server() {
	statusModes.resize(2);
}

void Server::handleCapab(std::vector<std::string> parsedLine) {
	// this will handle the 005 capab lines in the future
}

std::vector<char> Server::prefixModes(bool symbols) {
	if (symbols)
		return statusModes[1];
	return statusModes[0];
}