#include "modules.h"
#include "connection.h"

ModuleInterface::ModuleInterface(std::string confdir, std::string confname, unsigned short debug) : debugLevel(debug), directory(confdir), configName(confname) {
	pthread_attr_init(&detachedState);
	pthread_attr_setdetachstate(&detachedState, PTHREAD_CREATE_DETACHED);
	pthread_create(&serverCheckThread, &detachedState, serverCheck_thread, this); // start thread that checks for disconnected servers
	ConfigReader config (confname, confdir);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.getServerConfig(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > moduleConf = config.getModConfig(true);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter) {
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
		if (loadModule(modConfIter->first, true)) {
			if (debugLevel >= 2)
				std::cout << "Module " << modConfIter->first << " loaded successfully." << std::endl;
		} else
			std::cout << "Module " << modConfIter->first << " failed to load." << std::endl; // debug level 1
	}
	
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
		modIter->second->onLoadComplete(); // call the onLoadComplete hook in modules when all modules are loaded
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); ++servConfIter) {
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servConfIter->first, servConfIter->second));
		connectServer(servConfIter->first);
	}
	
	moduleConf = config.getModConfig(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConfIter = moduleConf.begin(); modConfIter != moduleConf.end(); ++modConfIter)
		moduleConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (modConfIter->first, modConfIter->second));
	serverConf = config.getServerConfig(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConf.begin(); servConfIter != serverConf.end(); ++servConfIter)
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servConfIter->first, servConfIter->second));
}

void ModuleInterface::sendToServer(std::string server, std::string rawLine) {
	std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return;
	serverIter->second->sendLine(rawLine);
}

std::tr1::unordered_map<std::string, std::string> ModuleInterface::getServerData(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::tr1::unordered_map<std::string, std::string> (); // a blank map for a nonexistent server
	return serverIter->second->getInfo();
}

std::vector<std::vector<char> > ModuleInterface::getServerChanModes(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::vector<std::vector<char> > (); // Empty structure for whoever can't check the server list for real servers
	return serverIter->second->getChanModes();
}

std::tr1::unordered_map<char, char> ModuleInterface::getServerPrefixes(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator serverIter = servers.find(server);
	if (serverIter == servers.end())
		return std::tr1::unordered_map<char, char> ();
	return serverIter->second->getPrefixes();
}

void ModuleInterface::callHook(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[1] == "PRIVMSG") {
		if (parsedLine[3][0] == (char)1) { // CTCP
			if (parsedLine[3][parsedLine[3].size()-1] == (char)1) // trim CTCP characters
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (parsedLine[3].substr(0, parsedLine[3].find_first_of(' ')) == "ACTION") {
				if (isChanType(parsedLine[2][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelAction(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				} else if (isChanType(parsedLine[2][1], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelAction(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onUserAction(server, parseNickFromHost(parsedLine[0]), parsedLine[3].substr(7));
				}
			} else {
				if (isChanType(parsedLine[2][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelCTCP(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
				} else if (isChanType(parsedLine[2][1], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelCTCP(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onUserCTCP(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
				}
			}
		} else {
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelMsg(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelMsg(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onUserMsg(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "NOTICE") {
		if (parsedLine[3][0] == (char)1) {
			if (parsedLine[3][parsedLine[3].size()-1] == (char)1)
				parsedLine[3] = parsedLine[3].substr(1, parsedLine[3].size()-2);
			else
				parsedLine[3] = parsedLine[3].substr(1);
			
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelCTCPReply(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelCTCPReply(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onUserCTCPReply(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		} else {
			if (isChanType(parsedLine[2][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelNotice(server, parsedLine[2], '0', parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else if (isChanType(parsedLine[2][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onChannelNotice(server, parsedLine[2].substr(1), parsedLine[2][0], parseNickFromHost(parsedLine[0]), parsedLine[3]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onUserNotice(server, parseNickFromHost(parsedLine[0]), parsedLine[3]);
			}
		}
	} else if (parsedLine[1] == "JOIN") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onChannelJoin(server, parsedLine[2], parsedLine[0]);
	} else if (parsedLine[1] == "PART") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		if (parsedLine.size() == 3) {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
				modIter->second->onChannelPart(server, parsedLine[2], parsedLine[0], "");
		} else {
			for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
				modIter->second->onChannelPart(server, parsedLine[2], parsedLine[0], parsedLine[3]);
		}
	} else if (parsedLine[1] == "QUIT") {
		if (parsedLine[0][0] == ':')
			parsedLine[0] = parsedLine[0].substr(1);
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onUserQuit(server, parsedLine[0], parsedLine[2]);
	} else if (parsedLine[1] == "NICK") {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onNickChange(server, parseNickFromHost(parsedLine[0]), parsedLine[2]);
	} else if (parsedLine[1] == "KICK") {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
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
				std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
				serverModes = servIter->second->getChanModes();
				prefixes = servIter->second->getPrefixes();
				short category;
				std::tr1::unordered_map<char, char>::iterator prefixIter = prefixes.find(parsedLine[3][i]);
				category = 0;
				if (prefixIter == prefixes.end()) {
					bool found = false;
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
					if (!found)
						category = 4;
				}
				
				if (category == 0 || category == 1 || (category == 2 && addMode)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelMode(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3][i], addMode, parsedLine[currParam]);
					currParam++;
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onChannelMode(server, parsedLine[2], parseNickFromHost(parsedLine[0]), parsedLine[3][i], addMode, "");
				}
			}
		}
	} else if (parsedLine[1].size() == 3 && charIsNumeric(parsedLine[1][0]) && charIsNumeric(parsedLine[1][1]) && charIsNumeric(parsedLine[1][2])) {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onNumeric(server, parsedLine[1], parsedLine);
	} else {
		for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
			modIter->second->onOtherData(server, parsedLine);
	}
}

void ModuleInterface::callHookOut(std::string server, std::vector<std::string> parsedLine) {
	if (parsedLine[0] == "PRIVMSG") {
		if (parsedLine[2][0] == (char)1) { // CTCP
			if (parsedLine[2][parsedLine.size()-1] == (char)1) // trim CTCP characters
				parsedLine[2] = parsedLine[2].substr(1,parsedLine[2].size()-2);
			else
				parsedLine[2] = parsedLine[2].substr(1);
			
			if (parsedLine[2].substr(0, parsedLine[2].find_first_of(' ')) == "ACTION") {
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutChannelAction(server, parsedLine[1], '0', parsedLine[2].substr(7));
				} else if (isChanType(parsedLine[1][1], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutChannelAction(server, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2].substr(7));
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutUserAction(server, parsedLine[1], parsedLine[2].substr(7));
				}
			} else {
				if (isChanType(parsedLine[1][0], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutChannelCTCP(server, parsedLine[1], '0', parsedLine[2]);
				} else if (isChanType(parsedLine[1][1], server)) {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutChannelCTCP(server, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
				} else {
					for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
						modIter->second->onOutUserCTCP(server, parsedLine[1], parsedLine[2]);
				}
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelMessage(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelMessage(server, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutUserMessage(server, parsedLine[1], parsedLine[2]);
			}
		}
	} else if (parsedLine[0] == "NOTICE") {
		if (parsedLine[2][0] == (char)1) { // CTCP reply
			if (parsedLine[2][parsedLine.size()-1] == (char)1) // trim both characters
				parsedLine[2] = parsedLine[2].substr(1, parsedLine[2].size()-2);
			else
				parsedLine[2] = parsedLine[2].substr(1);
			
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelCTCPReply(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelCTCPReply(server, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutUserCTCPReply(server, parsedLine[1], parsedLine[2]);
			}
		} else {
			if (isChanType(parsedLine[1][0], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelNotice(server, parsedLine[1], '0', parsedLine[2]);
			} else if (isChanType(parsedLine[1][1], server)) {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutChannelNotice(server, parsedLine[1].substr(1), parsedLine[1][0], parsedLine[2]);
			} else {
				for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
					modIter->second->onOutUserNotice(server, parsedLine[1], parsedLine[2]);
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

bool ModuleInterface::isChanType(char chanPrefix, std::string server) {
	if (servers.find(server) == servers.end())
		return false;
	std::vector<char> prefixes = servers.find(server)->second->getChanTypes();
	for (unsigned int i = 0; i < prefixes.size(); i++) {
		if (chanPrefix == prefixes[i])
			return true;
	}
	return false;
}

void ModuleInterface::rehash() {
	ConfigReader config (configName, directory);
	serverConfigs.clear();
	serverConfigs = config.getServerConfig(true);
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> > serverConf = config.getServerConfig(false);
	for (std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servIter = serverConf.begin(); servIter != serverConf.end(); ++servIter)
		serverConfigs.insert(std::pair<std::string, std::tr1::unordered_map<std::string, std::string> > (servIter->first, servIter->second));
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

bool ModuleInterface::connectServer(std::string serverName) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator servConfIter = serverConfigs.find(serverName);
	if (servConfIter == serverConfigs.end())
		return false;
	servers.insert(std::pair<std::string, Server*> (serverName, new Server (serverName, servConfIter->second, this, debugLevel)));
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
		modIter->second->onConnect(serverName); // call onConnect hook
	return true;
}

bool ModuleInterface::loadModule(std::string modName, bool startup) {
	std::tr1::unordered_map<std::string, std::tr1::unordered_map<std::string, std::string> >::iterator modConf = moduleConfigs.find(modName);
	if (modConf == moduleConfigs.end())
		return false;
	std::string fileLoc = directory + "/modules/" + modName;
	void* openModule = dlopen(fileLoc.c_str(), RTLD_LAZY);
	if (openModule == NULL) {
		std::string error = "Could not load module " + modName + ": " + dlerror();
		std::perror(error.c_str()); // debug level 1
		return false;
	}
	typedef void* (*module_spawn_t)();
	module_spawn_t spawnModule = (module_spawn_t) dlsym(openModule, "spawn");
	const char* dlsymError = dlerror();
	if (dlsymError) {
		std::string error = "Could not load module " + modName + ": " + dlsymError;
		std::perror(error.c_str()); // debug level 1
		return false;
	}
	
	Module* newModule = (Module*)spawnModule();
	if (newModule->botAPIversion() != 1002) { // compare to current API version
		dlclose(openModule);
		std::cout << "Module " << modName << " is not compatible with the current API." << std::endl; // debug level 1
		return false;
	}
	newModule->init(modConf->second, this, modName, debugLevel);
	modules.insert(std::pair<std::string, Module*> (modName, newModule));
	moduleFiles.insert(std::pair<std::string, void*> (modName, openModule));
	std::vector<std::string> abilities = newModule->getAbilities();
	for (unsigned int i = 0; i < abilities.size(); i++)
		modAbilities.insert(std::pair<std::string, std::string> (abilities[i], modName));
	if (!startup)
		return newModule->onLoadComplete();
	return true;
}

void ModuleInterface::unloadModule(std::string modName) {
	if (modules.find(modName) == modules.end())
		return;
	moduleToUnload.push_back(modName);
	pthread_t tum;
	pthread_create(&tum, NULL, tUnloadMod_thread, this);
}

void* ModuleInterface::tUnloadMod_thread(void* mip) {
	ModuleInterface* modi = (ModuleInterface*) mip;
	modi->tUnloadMod();
	return NULL;
}

void ModuleInterface::tUnloadMod() {
	if (moduleToUnload.empty())
		return;
	sleep(1);
	std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.find(moduleToUnload[0]);
	std::tr1::unordered_map<std::string, void*>::iterator modFileIter = moduleFiles.find(moduleToUnload[0]);
	if (!modIter->second->getAbilities().empty()) {
		for (std::multimap<std::string, std::string>::iterator modAbleIter = modAbilities.begin(); modAbleIter != modAbilities.end(); ++modAbleIter) {
			if (modAbleIter->second == modIter->first) {
				modAbilities.erase(modAbleIter);
				--modAbleIter;
			}
		}
	}
	delete modIter->second;
	modules.erase(modIter);
	dlclose(modFileIter->second);
	moduleFiles.erase(modFileIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator moduleIter = modules.begin(); moduleIter != modules.end(); ++moduleIter)
		moduleIter->second->onRehash(); // provide modules with a way to detect unloading
	moduleToUnload.erase(moduleToUnload.begin()); // remove first element, the one we just removed
}

void ModuleInterface::removeServer(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	delete servIter->second;
	servers.erase(servIter);
	for (std::tr1::unordered_map<std::string, Module*>::iterator modIter = modules.begin(); modIter != modules.end(); ++modIter)
		modIter->second->onQuit(server); // call onQuit hook
}

std::tr1::unordered_map<std::string, Module*> ModuleInterface::getModules() {
	return modules;
}

std::list<std::string> ModuleInterface::getServers() {
	std::list<std::string> serverList;
	for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter)
		serverList.insert(serverList.end(), servIter->first);
	return serverList;
}

std::multimap<std::string, std::string> ModuleInterface::getModuleAbilities() {
	return modAbilities;
}

std::list<std::string> ModuleInterface::getChannels(std::string server) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->getChannels();
}

std::string ModuleInterface::getChannelTopic(std::string server, std::string channel) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->getChannelTopic(channel);
}

std::list<std::string> ModuleInterface::getChannelUsers(std::string server, std::string channel) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> (); // return empty list to those who cannot provide a valid server name
	return servIter->second->getChannelUsers(channel);
}

std::pair<char, char> ModuleInterface::getUserStatus(std::string server, std::string channel, std::string user) {
	std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<char, char> ('0', ' '); // pair for normal user
	return servIter->second->getUserStatus(channel, user);
}

void* ModuleInterface::serverCheck_thread(void* ptr) {
	ModuleInterface* modi = (ModuleInterface*) ptr;
	modi->serverCheck();
	return NULL;
}

void ModuleInterface::serverCheck() {
	while (true) {
		sleep(60); // one minute pause between checks
		for (std::tr1::unordered_map<std::string, Server*>::iterator servIter = servers.begin(); servIter != servers.end(); ++servIter) {
			if (!servIter->second->stillConnected()) {
				delete servIter->second;
				if (debugLevel >= 2)
					std::cout << servIter->second << " lost connection.  Reconnecting..." << std::endl;
				servIter->second = new Server(servIter->first, serverConfigs[servIter->first], this, debugLevel); // make new server for reconnecting
			}
		}
	}
}