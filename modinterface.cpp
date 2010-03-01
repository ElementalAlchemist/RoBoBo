#include "modules.h"

#ifndef MODINTERFACE_CPP
#define MODINTERFACE_CPP
ModuleInterface::ModuleInterface(std::string confdir, std::string confname, unsigned short debug) : debugLevel(debug), directory(confdir), configName(confname) {
	ConfigReader config (confname, confdir);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.getServerConfig();
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.getModConfig(true);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter) {
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
		if (loadModule(modConfIter->first, true))
			std::cout << "Module " << modConfIter->first << " loaded successfully." << std::endl;
		else
			std::cout << "Module " << modConfIter->first << " failed to load." << std::endl;
	}
	std::vector<std::string> abilities;
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++) {
		abilities = modIter->second->getAbilities();
		for (unsigned int i = 0; i < abilities.size(); i++)
			modAbilities.insert(std::pair<std::string, std::string> (abilities[i], modIter->first));
	}
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
		modIter->second->onLoadComplete(); // call the onLoadComplete hook in modules when all modules are loaded
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); servConfIter++)
		connectServer(servConfIter->first, servConfIter->second);
	
	moduleConf = config.getModConfig(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter)
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
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

std::vector<std::vector<char> > ModuleInterface::getServerChanModes(std::string server) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.begin(); serverIter != servers.end(); serverIter++) {
		if (serverIter->first == server)
			return serverIter->second->getChanModes();
	}
	std::vector<std::vector<char> > data; // If you're going to send random channel mode requests, at least look through the server list
	return data; // I mean, seriously.  What's wrong with you?
}

std::tr1::unordered_map<char, char> ModuleInterface::getServerPrefixes(std::string server) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.begin(); serverIter != servers.end(); serverIter++) {
		if (serverIter->first == server)
			return serverIter->second->getPrefixes();
	}
	std::tr1::unordered_map<char, char> data; // I'm tired of saying it.  You're probably handling this from a hook.  USE THE SERVER PARAMETER.
	return data; // It doesn't get any easier.
}

void ModuleInterface::callHook(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "PRIVMSG") {
		if (parsedLine[3][0] == (char)1) { // CTCP
			if (parsedLine[3][parsedLine[3].size()-1] == (char)1) // trim CTCP characters
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (parsedLine[3].substr(0, parsedLine[3].find_first_of(' ')) == "ACTION") {
				if (isChanType(parsedLine[2][0])) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onChannelAction(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				} else if (isChanType(parsedLine[2][1])) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onChannelAction(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onUserAction(server, parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				}
			} else {
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
			if (parsedLine[3][parsedLine[3].size()-1] == (char)1)
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
	} else if (parsedLine[1] == "NICK") {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onNickChange(server, parseNickFromHost(parsedLine[0]), parsedLine[2]);
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
				std::tr1::unordered_map<char, char> prefixes;
				for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); servIter++) {
					if (servIter->first == server) {
						serverModes = servIter->second->getChanModes();
						prefixes = servIter->second->getPrefixes();
					}
				}
				bool found = false;
				short category;
				for (std::tr1::unordered_map<char, char>::iterator prefixIter = prefixes.begin(); prefixIter != prefixes.end(); ++prefixIter) {
					if (parsedLine[3][i] == prefixIter->first) {
						found = true;
						category = 0;
						break;
					}
				}
				if (!found) {
					for (unsigned int j = 0; j < serverModes.size(); j++) {
						for (unsigned int k = 0; k < serverModes[j].size(); k++) {
							if (parsedLine[3][i] == serverModes[j][k]) {
								found = true;
								category = j;
								break;
							}
						}
						if (found)
							break;
					}
				}
				if (!found)
					category = 4;
				
				if (category == 0 || category == 1 || (category == 2 && addMode)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onChannelMode(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3][i], addMode, parsedLine[currParam]);
					currParam++;
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
			
			if (parsedLine[3].substr(0, parsedLine[3].find_first_of(' ')) == "ACTION") {
				if (isChanType(parsedLine[2][0])) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onOutChannelAction(server, parsedLine[2], '0', parsedLine[3].substr(7));
				} else if (isChanType(parsedLine[2][1])) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onOutChannelAction(server, parsedLine[2].substr(1), parsedLine[2][0], parsedLine[3].substr(7));
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); modIter++)
						modIter->second->onOutUserAction(server, parsedLine[2], parsedLine[3].substr(7));
				}
			} else {
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

void ModuleInterface::rehash() {
	ConfigReader config (configName, directory);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.getServerConfig();
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servIter = serverConf.begin(); servIter != serverConf.end(); ++servIter) {
		if (servers.find(servIter->first) == servers.end())
			connectServer(servIter->first, servIter->second);
	}
	moduleConfigs.clear();
	moduleConfigs = config.getModConfig(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.getModConfig(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIter = moduleConf.begin(); modIter != moduleConf.end(); ++modIter)
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modIter->first, modIter->second));
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modIter = moduleConfigs.begin(); modIter != moduleConfigs.end(); ++modIter) {
		std::tr1::unordered_map<std::string, Module*>::iterator module = modules.find(modIter->first);
		if (module != modules.end()) {
			module->second->reconf(modIter->second);
			module->second->onRehash();
		}
	}
}

void ModuleInterface::connectServer(std::string serverName, std::tr1::unordered_map<std::string, std::string> serverConf) {
	servers.insert(std::pair<std::string, Server*> (serverName, new Server (serverName, serverConf, this)));
}

bool ModuleInterface::loadModule(std::string modName, bool startup) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConf = moduleConfigs.find(modName);
	if (modConf == moduleConfigs.end())
		return false;
	std::string fileLoc = directory + "/modules/" + modName;
	void* openModule = dlopen(fileLoc.c_str(), RTLD_LAZY);
	if (openModule == NULL) {
		std::string error = "Could not load module " + modName + ": " + dlerror();
		std::perror(error.c_str());
		return false;
	}
	char* dlsymError;
	typedef void* (*module_spawn_t)();
	module_spawn_t spawnModule = (module_spawn_t) dlsym(openModule, "spawn");
	dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not load module " + modName + ": " + dlsymError;
		std::perror(error.c_str());
		return false;
	}
	
	Module* newModule = (Module*)spawnModule();
	newModule->init(modConf->second, this, modName);
	modules.insert(std::pair<std::string, Module*> (modName, newModule));
	moduleFiles.insert(std::pair<std::string, void*> (modName, openModule));
	if (!startup)
		newModule->onLoadComplete();
	return true;
}

void ModuleInterface::unloadModule(std::string modName) {
	std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(modName);
	std::tr1::unordered_map<std::string, void*>::iterator modFileIter = moduleFiles.find(modName);
	if (modIter == modules.end())
		return;
	delete modIter->second;
	modules.erase(modIter);
	dlclose(modFileIter->second);
	moduleFiles.erase(modFileIter);
}

void ModuleInterface::removeServer(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	delete servIter->second;
	servers.erase(servIter);
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

std::multimap<std::string, std::string> ModuleInterface::getModuleAbilities() {
	return modAbilities;
}

std::list<std::string> ModuleInterface::getChannels(std::string server) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
		if (servIter->first == server)
			return servIter->second->getChannels();
	}
	return std::list<std::string> ();
}

std::string ModuleInterface::getChannelTopic(std::string server, std::string channel) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
		if (servIter->first == server)
			return servIter->second->getChannelTopic(channel);
	}
	return "";
}

std::list<std::string> ModuleInterface::getChannelUsers(std::string server, std::string channel) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
		if (servIter->first == server)
			return servIter->second->getChannelUsers(channel);
	}
	return std::list<std::string> (); // return an empty list to those who can't provide a correct server name.
}

std::pair<char, char> ModuleInterface::getUserStatus(std::string server, std::string channel, std::string user) {
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
		if (servIter->first == server)
			return servIter->second->getUserStatus(channel, user);
	}
	return std::pair<char, char> ('0', ' '); // if we're here return the pair for a statusless user
}
#endif