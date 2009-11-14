#include "robobo.h"

std::tr1::unordered_map<std::string, Server> connectedServers;
std::list<std::string> serverList, moduleList;

std::list<std::string> makeServerList(ConfigReader& config) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConfig = config.getServerConfig();
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator serverIterator;
	for (serverIterator = serverConfig.begin(); serverIterator != serverConfig.end(); serverIterator++) {
		connectedServers[serverIterator->first] = Server(serverIterator->first, serverIterator->second);
		serverList.insert(serverList.end(), serverIterator->first);
	}
	return serverList;
}

int main(int argc, char** argv) {
	ConfigReader config;
	serverList = makeServerList(config);
}