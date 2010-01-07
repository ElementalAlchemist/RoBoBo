#include "modules.h"

#ifndef MODINTERFACE_CPP
#define MODINTERFACE_CPP
ModuleInterface::ModuleInterface(std::string confdir, std::string confname) {
	ConfigReader config (confname, confdir);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.getServerConfig();
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.getModConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); modConfIter++)
		loadModule(modConfIter->first, modConfIter->second);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); servConfIter++)
		connectServer(servConfIter->first, servConfIter->second);
}

void ModuleInterface::sendToServer(std::string server, std::string rawLine) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.begin(); serverIter != servers.end(); serverIter++) {
		if (serverIter->first == server)
			serverIter->second->sendLine(rawLine);
	}
}

std::tr1::unordered_map<std::string, std::string> ModuleInterface::getServerData(std::string server) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.begin(); serverIter != servers.end(); serverIter++) {
		if (serverIter->first == server)
			return serverIter->second->getInfo();
	}
	std::tr1::unordered_map<std::string, std::string> data; // if we're this far the module coder probably shouldn't be coding modules
	return data; // but here's a blank map, just for you. :)
}

void ModuleInterface::callHook(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "PRIVMSG") {
		if (parsedLine[3][0] == (char)1) { // CTCP
			if (parsedLine[3][parsedLine.size()-1] == (char)1) // trim CTCP characters
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelCTCP(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelCTCP(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onUserCTCP(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelMsg(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelMsg(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onUserMsg(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		if (parsedLine[3][0] == (char)1) {
			if (parsedLine[3][parsedLine.size()-1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelCTCPReply(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelCTCPReply(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onUserCTCPReply(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelNotice(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onChannelNotice(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onUserNotice(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "JOIN") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onChannelJoin(server, parsedLine[2], parsedLine[0]);
	} else if (parsedLine[1] == "PART") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onChannelPart(server, parsedLine[2], parsedLine[0], parsedLine[3]);
	} else if (parsedLine[1] == "QUIT") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onUserQuit(server, parsedLine[0], parsedLine[2]);
	} else if (parsedLine[1] == "KICK") {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onChannelKick(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3], parsedLine[4]);
	} else if (parsedLine[1] == "MODE") {
		bool addMode = true;
		int currParam = 4;
		for (unsigned int i = 0; i < parsedLine[3].size(); i++) {
			if (parsedLine[3][i] == '+')
				addMode = true;
			else if (parsedLine[3][i] == '-')
				addMode = false;
			else {
				std::vector<std::vector<char> > serverModes;
				for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); servIter++) {
					if (servIter->first == server)
						serverModes = servIter->second->getChanModes();
				}
				bool found = false;
				short category;
				for (unsigned int j = 0; j < serverModes[0].size(); j++) {
					if (parsedLine[3][i] == serverModes[0][j]) {
						found = true;
						category = 0;
						break;
					}
				}
				if (!found) {
					for (unsigned int j = 0; j < serverModes[1].size(); j++) {
						if (parsedLine[3][i] == serverModes[1][j]) {
							found = true;
							category = 1;
							break;
						}
					}
				}
				if (!found) {
					for (unsigned int j = 0; j < serverModes[2].size(); j++) {
						if (parsedLine[3][i] == serverModes[1][j]) {
							found = true;
							category = 2;
							break;
						}
					}
				}
				if (!found) {
					for (unsigned int j = 0; j < serverModes[3].size(); j++) {
						if (parsedLine[3][i] == serverModes[1][j]) {
							found = true;
							category = 3;
						}
					}
				}
				if (!found)
					category = 4;
				if (category == 0 || category == 1 || (category == 2 && addMode)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onChannelMode(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3][i], addMode, parsedLine[currParam++]);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onChannelMode(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3][i], addMode, "");
				}
			}
		}
	} else if (parsedLine[1].size() == 3 && charIsNumeric(parsedLine[1][0]) && charIsNumeric(parsedLine[1][1]) && charIsNumeric(parsedLine[1][2])) {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onNumeric(server, parsedLine[1], parsedLine);
	} else {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
			modIter->second->onOtherData(server, parsedLine);
	}
}

void ModuleInterface::callHookOut(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "PRIVMSG") {
		if (parsedLine[3][0] == (char)1) { // CTCP
			if (parsedLine[3][parsedLine.size()-1] == (char)1) // trim CTCP characters
				parsedLine[3] = parsedLine[3].substr(1,parsedLine.size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelCTCP(server, parsedLine[2], '0', parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelCTCP(server, parsedLine[2].substr(1), parsedLine[2][0], parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutUserCTCP(server, parsedLine[2], parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelMessage(server, parsedLine[2], '0', parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelMessage(server, parsedLine[2].substr(1), parsedLine[2][0], parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutUserMessage(server, parsedLine[2], parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		if (parsedLine[3][0] == (char)1) { // CTCP reply
			if (parsedLine[3][parsedLine.size()-1] == (char)1) // trim both characters
				parsedLine[3] = parsedLine[3].substr(1, parsedLine.size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelCTCPReply(server, parsedLine[2], '0', parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelCTCPReply(server, parsedLine[2].substr(1), parsedLine[2][0], parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutUserCTCPReply(server, parsedLine[2], parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelNotice(server, parsedLine[2], '0', parsedLine[3]);
			} else if (isChanType(parsedLine[2][1])) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutChannelNotice(server, parsedLine[2].substr(1), parsedLine[2][0], parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
					modIter->second->onOutUserNotice(server, parsedLine[2], parsedLine[3]);
			}
		}
	}
}

std::string ModuleInterface::parseNickFromHost(std::string host) {
	if (host[0] == ':')
		host = host.substr(1);
	return host.substr(0, host.find_first_of('!'));
}

bool ModuleInterface::charIsNumeric(char number) {
	if (number == '0' || number == '1' || number == '2' || number == '3' || number == '4' || number == '5' || number == '6' || number == '7' || number == '8' || number == '9')
		return true;
	return false;
}

bool ModuleInterface::isChanType(char chanPrefix) {
	std::vector<char> prefixes;
	for (std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.begin(); serverIter != servers.end(); serverIter++) {
		prefixes = serverIter->second->getChanTypes();
		for (unsigned int i = 0; i < prefixes.size(); i++) {
			if (chanPrefix == prefixes[i])
				return true;
		}
	}
	return false;
}

void ModuleInterface::connectServer(std::string serverName, std::tr1::unordered_map<std::string, std::string> serverConf) {
	servers.insert(std::pair<std::string, Server*> (serverName, new Server (serverName, serverConf, this)));
}

void ModuleInterface::loadModule(std::string modName, std::tr1::unordered_map<std::string, std::string> modConf) {
	std::string fileLoc = "modules/" + modName;
	void* openModule = dlopen(fileLoc.c_str(), RTLD_LAZY);
	if (openModule == NULL) {
		std::string error = "Could not load module " + modName + ": " + dlerror();
		std::perror(error.c_str());
		return;
	}
	char* dlsymError;
	Module* spawnModule = (Module*) dlsym(openModule, "spawn");
	dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not load module " + modName + ": " + dlsymError;
		std::perror(error.c_str());
		return;
	}
	
	Module* newModule = spawnModule;
	newModule->init(modConf, this);
	modules.insert(std::pair<std::string, Module*> (modName, newModule));
	newModule->onPostInit();
}

std::tr1::unordered_map<std::string, Module*> ModuleInterface::getModules() {
	return modules;
}

std::list<std::string> ModuleInterface::getServers() {
	std::list<std::string> serverList;
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); servIter++)
		serverList.insert(serverList.end(), servIter->first);
	return serverList;
}
#endif