#include "modinterface.h"

#ifndef MODIFACE_ROBOBO
#define MODIFACE_ROBOBO
class ModuleInterface {
	public:
		ModuleInterface(std::tr1::unordered_map<std::string, Server>* serverMap, std::tr1::unordered_map<std::string, Module>* moduleMap);
		void sendToServer(std::string server, std::string rawLine);
		void callHook(std::vector<std::string> parsedLine);
	private:
		std::tr1::unordered_map<std::string, Server>* servers;
		std::tr1::unordered_map<std::string, Module>* modules;
		std::string parseNickFromHost(std::string host);
};

ModuleInterface::ModuleInterface(std::tr1::unordered_map<std::string, Server>* serverMap, std::tr1::unordered_map<std::string, Module>* moduleMap) {
	servers = serverMap;
	modules = moduleMap;
}

void ModuleInterface::sendToServer(std::string server, std::string rawLine) {
	for (std::tr1::unordered_map<std::string, Server>::iterator serverIter = servers->begin(); serverIter != servers->end(); serverIter++) {
		if (serverIter->first == server)
			serverIter->second.sendLine(rawLine);
	}
}

void ModuleInterface::callHook(std::vector<std::string> parsedLine) {
	// comment on what the command (parsedLine[1]) is
}

std::string ModuleInterface::parseNickFromHost(std::string host) {
	if (host[0] == ':')
		host = host.substr(1);
	return host.substr(0, host.find_first_of('!'));
}
#endif