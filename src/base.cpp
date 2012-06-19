#include "base.h"

Base::Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log) : workingDir(working), configDir(config), configName(configFileName), debugLevel(debug), logDump(log), startup(true) {}

void Base::readConfiguration() {
	std::string fileName = configDir + "/" + configName;
	std::ifstream configFile (fileName.c_str());
	bool readingType, readingName, readingBlock, readingKey, readingValue, valueInQuotes, valueConcatted, valueVar, escapedValue;
	std::string currentBlockType, currentBlockName, currentKey, currentValue, currentValueVar;
	std::list<std::string> includeFiles;
	std::map<std::string, std::string> currentBlock;
	size_t lineNum = 1;
	while (configFile.good()) {
		char confChar = configFile.get();
		if (confChar == '\n')
			lineNum++;
		if (confChar == '#' && !valueInQuotes) {
			while (configFile.good() && confChar != '\n')
				confChar = configFile.get();
			lineNum++;
			continue;
		}
		if (confChar == ' ' || confChar == '\t' || confChar == '\r' || confChar == '\n') {
			if (readingType) {
				readingType = false;
				if (currentBlockType != "server" && currentBlockType != "serverconf" && currentBlockType != "module" && currentBlockType != "moduleconf" && currentBlockType != "include")
					std::cerr << "An invalid block type of " << currentBlockType << " was found on line " << lineNum << "." << std::endl;
			}
			if (readingName)
				readingName = false;
			else if (readingKey)
				readingKey = false;
			else if (valueVar) {
				valueVar = false;
				valueConcatted = false;
			} else if (valueInQuotes)
				currentValue += confChar;
			continue;
		}
		if (confChar == ';') {
			if (!readingBlock && currentBlockType == "include") {
				includeFiles.push_back(currentBlockName);
				currentBlockType = "";
				currentBlockName = "";
				readingName = false;
			} else if (readingValue && !valueInQuotes && !valueConcatted) {
				if (valueVar || currentValueVar != "") {
					currentValue += currentBlock[currentValueVar];
					currentValueVar = "";
				}
				currentBlock.insert(std::pair<std::string, std::string> (currentKey, currentValue));
				currentKey = "";
				currentValue = "";
			} else if (valueInQuotes)
				currentValue += ";";
			else
				std::cerr << "Invalid semicolon found on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '{') {
			if (!readingName && currentBlockName == "") {
				std::cerr << "A block name was not given for the block starting on line " << lineNum << "." << std::endl;
				currentBlockType = "";
			} else if (readingName || (currentBlockName != "" && !readingBlock)) {
				readingName = false;
				readingBlock = true;
			} else if (valueInQuotes)
				currentValue += "{";
			else
				std::cerr << "Unexpected opening brace ('{') encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '}') {
			if (valueInQuotes)
				currentValue += "}";
			else if (readingBlock && !readingKey && !readingValue) {
				if (currentBlockType == "server") {
					serverConfig.insert(std::pair<std::string, std::map<std::string, std::string>> (currentBlockName, currentBlock));
					startupServers.push_back(currentBlockName);
				} else if (currentBlockType == "serverconf")
					serverConfig.insert(std::pair<std::string, std::map<std::string, std::string>> (currentBlockName, currentBlock));
				else if (currentBlockType == "module") {
					moduleConfig.insert(std::pair<std::string, std::map<std::string, std::string>> (currentBlockName, currentBlock));
					startupModules.push_back(currentBlockName);
				} else if (currentBlockType == "moduleconf")
					moduleConfig.insert(std::pair<std::string, std::map<std::string, std::string>> (currentBlockName, currentBlock));
				else // Throw up another error about the incorrectly typed block to make sure they absolutely know what it is.
					std::cerr << "The invalid block ends on line " << lineNum << "." << std::endl;
				readingBlock = false;
				currentBlock.clear();
				currentBlockType = "";
				currentBlockName = "";
			} else
				std::cerr << "Unexpected closing brace ('}') encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '\\') {
			if (valueInQuotes && escapedValue)
				currentValue += "\\";
			else if (valueInQuotes)
				escapedValue = true;
			else
				std::cerr << "Unexpected backslash encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '"') {
			if (readingValue && !valueInQuotes && valueConcatted) {
				valueInQuotes = true;
				valueConcatted = false;
			} else if (valueInQuotes) {
				if (escapedValue)
					currentValue += "\"";
				else
					valueInQuotes = false;
			} else
				std::cerr << "Unexpected quote mark encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '=') {
			if (readingBlock && (readingKey || (currentValue == "" && currentKey != ""))) {
				readingKey = false;
				readingValue = true;
				valueConcatted = true;
			} else if (valueInQuotes)
				currentValue += "=";
			else
				std::cerr << "Unexpected equals sign encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		if (confChar == '+') {
			if (readingValue && !valueInQuotes && !valueConcatted) {
				if (valueVar || currentValueVar != "")
					currentValue += currentBlock[currentValueVar];
				valueConcatted = true;
			} else if (valueInQuotes)
				currentValue += "+";
			else
				std::cerr << "Unexpected plus sign encountered on line " << lineNum << "." << std::endl;
			continue;
		}
		escapedValue = false;
		if (!readingBlock && !readingType && !readingName) {
			if (currentBlockType == "") {
				readingType = true;
				currentBlockType += confChar;
			} else if (currentBlockName == "") {
				readingName = true;
				currentBlockName += confChar;
			} else
				std::cerr << "Unexpected character ('" << confChar << "') encountered on line " << lineNum << "." << std::endl;
		} else if (readingType)
			currentBlockType += confChar;
		else if (readingName)
			currentBlockName += confChar;
		else if (readingBlock) {
			if (!readingKey && !readingValue) {
				readingKey = true;
				currentKey += confChar;
			} else if (readingKey)
				currentKey += confChar;
			else if (valueInQuotes)
				currentValue += confChar;
			else if (valueVar)
				currentValueVar += confChar;
		}
	}
	if (readingBlock)
		std::cerr << "End of file reached in the middle of a block." << std::endl;
}

void Base::loadModules() {
	for (std::string modName : startupModules)
		loadModule(modName);
	// Because this function is called at startup, also run through the requires/supports and onLoadCompletes
	std::list<std::string> unloadList; // Keep a list of modules to unload at the end to keep the loop sane
	for (std::pair<std::string, Module*> module : highModules) {
		if (!completeStartupLoad(module.first, module.second))
			unloadList.push_back(module.first);
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (!completeStartupLoad(module.first, module.second))
			unloadList.push_back(module.first);
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (!completeStartupLoad(module.first, module.second))
			unloadList.push_back(module.first);
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (!completeStartupLoad(module.first, module.second))
			unloadList.push_back(module.first);
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (!completeStartupLoad(module.first, module.second))
			unloadList.push_back(module.first);
	}
	for (std::string modToUnload : unloadList)
		unloadModule(modToUnload, false);
}

bool Base::completeStartupLoad(std::string modName, Module* modptr) {
	std::list<std::string> modRequires = modptr->requires();
	for (std::string requirement : modRequires) {
		if (moduleServices[requirement].empty()) {
			std::cerr << "Module " << modName << " requires the service " << requirement << ", which is not provided by another module." << std::endl;
			return false;
		}
		moduleSupports[requirement].push_back(modName);
	}
	std::list<std::string> modSupports = modptr->supports();
	for (std::string supporting : modSupports)
		moduleSupports[supporting].push_back(modName);
	if (modptr->onLoadComplete())
		return true; // Also since it's successful, tell the loadModules loop that all is well
	// Or if it's not successful, tell it that all is not well
	return false;
}

void Base::connectServers() {
	for (std::string serverName : startupServers)
		connectServer(serverName);
}

void Base::checkServers() {
	// TODO: server check
}

void Base::unloadEverything() {
	while (!servers.empty())
		disconnectServer(servers.begin()->first);
	// Every module is represented in moduleFiles, so just use that to unload all the modules
	while (!moduleFiles.empty())
		unloadModule(moduleFiles.begin()->first, true);
}

LoadResult Base::loadModule(std::string modName) {
	if (moduleFiles.find(modName) != moduleFiles.end())
		return LOAD_ALREADYLOADED; // Do not attempt to load a module if it's already here
	// Modules will be in the modules directory and have the m_ prefix
	std::string fileName = workingDir + "/modules/m_" + modName + ".so";
	void* modFile = dlopen(fileName.c_str(), RTLD_NOW);
	const char* fileOpenError = dlerror();
	if (fileOpenError != NULL) {
		std::cerr << "Module " << modName << " could not be opened: " << fileOpenError << std::endl;
		return LOAD_OPEN_ERROR;
	}
	// The spawn function of modules returns a module instance that we can use
	module_spawn_t* moduleSpawn = (module_spawn_t*) dlsym(modFile, "spawn");
	const char* spawnError = dlerror();
	if (spawnError != NULL) {
		std::cerr << "Spawn not found in module " << modName << ": " << spawnError << std::endl;
		dlclose(modFile);
		return LOAD_OPEN_ERROR;
	}
	Module* newModule = (Module*) moduleSpawn(modName, moduleConfig[modName], workingDir, debugLevel, this);
	if (newModule->apiVersion() != 3000) {
		std::cerr << "Module " << modName << " is not compatible with this version of RoBoBo." << std::endl;
		delete newModule;
		dlclose(modFile);
		return LOAD_INCOMPATIBLE;
	}
	// Add the module to the appropriate module list according to its priority
	modulePriority.insert(std::pair<std::string, Priority> (modName, newModule->priority));
	switch (newModule->priority) {
		case PRI_HIGH:
			highModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case PRI_MEDIUM_HIGH:
			mediumHighModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case PRI_NORMAL:
			normalModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case PRI_MEDIUM_LOW:
			mediumLowModules.insert(std::pair<std::string, Module*> (modName, newModule));
			break;
		case PRI_LOW:
			lowModules.insert(std::pair<std::string, Module*> (modName, newModule));
	}
	moduleFiles.insert(std::pair<std::string, void*> (modName, modFile));
	// Save the module name in the list of each service it provides
	std::list<std::string> modProvides = newModule->provides();
	for (std::string provided : modProvides)
		moduleServices[provided].push_back(modName);
	moduleDescriptions.insert(std::pair<std::string, std::string> (modName, newModule->description()));
	if (!startup) {
		std::list<std::string> modRequires = newModule->requires();
		for (std::string requirement : modRequires) {
			// Check that the module's requirements are met
			if (moduleServices[requirement].empty()) {
				std::cerr << "Module " << modName << " requires the service " << requirement << ", which is not provided by another module." << std::endl;
				unloadModule(modName, false);
				return LOAD_NODEPENDS;
			}
			moduleRequires[requirement].push_back(modName);
			moduleSupports[requirement].push_back(modName);
		}
		std::list<std::string> modSupports = newModule->supports();
		for (std::string supporting : modSupports)
			moduleSupports[supporting].push_back(modName);
		if (newModule->onLoadComplete()) {
			// Since it's successful, call the hook in other modules for this module being loaded
			for (std::pair<std::string, Module*> module : highModules) {
				if (module.first != modName)
					module.second->onModuleLoad(modName);
			}
			for (std::pair<std::string, Module*> module : mediumHighModules) {
				if (module.first != modName)
					module.second->onModuleLoad(modName);
			}
			for (std::pair<std::string, Module*> module : normalModules) {
				if (module.first != modName)
					module.second->onModuleLoad(modName);
			}
			for (std::pair<std::string, Module*> module : mediumLowModules) {
				if (module.first != modName)
					module.second->onModuleLoad(modName);
			}
			for (std::pair<std::string, Module*> module : lowModules) {
				if (module.first != modName)
					module.second->onModuleLoad(modName);
			}
		} else {
			unloadModule(modName, false);
			return LOAD_FAILURE;
		}
	}
	return LOAD_SUCCESS;
}

void Base::unloadModule(std::string modName, bool wasLoaded) {
	if (moduleFiles.find(modName) == moduleFiles.end())
		return; // Do not try to unload the module if it's not currently loaded
	if (wasLoaded) {
		// Call the onUnload hook so this module can clean up after itself
		switch (modulePriority[modName]) {
			case PRI_HIGH:
				highModules.find(modName)->second->onUnload();
				break;
			case PRI_MEDIUM_HIGH:
				mediumHighModules.find(modName)->second->onUnload();
				break;
			case PRI_NORMAL:
				normalModules.find(modName)->second->onUnload();
				break;
			case PRI_MEDIUM_LOW:
				mediumLowModules.find(modName)->second->onUnload();
				break;
			case PRI_LOW:
				lowModules.find(modName)->second->onUnload();
		}
		// Call the hook for unloading this module in all other modules
		for (std::pair<std::string, Module*> module : highModules) {
			if (module.first != modName)
				module.second->onModuleUnload(modName);
		}
		for (std::pair<std::string, Module*> module : mediumHighModules) {
			if (module.first != modName)
				module.second->onModuleUnload(modName);
		}
		for (std::pair<std::string, Module*> module : normalModules) {
			if (module.first != modName)
				module.second->onModuleUnload(modName);
		}
		for (std::pair<std::string, Module*> module : mediumLowModules) {
			if (module.first != modName)
				module.second->onModuleUnload(modName);
		}
		for (std::pair<std::string, Module*> module : lowModules) {
			if (module.first != modName)
				module.second->onModuleUnload(modName);
		}
	}
	for (std::pair<std::string, std::list<std::string>> service : moduleServices)
		service.second.remove(modName);
	for (std::pair<std::string, std::list<std::string>> service : moduleRequires)
		service.second.remove(modName);
	for (std::pair<std::string, std::list<std::string>> service : moduleSupports)
		service.second.remove(modName);
	std::map<std::string, Module*>::iterator modEntry;
	// Remove the module from whatever list it's in, based on its priority
	switch (modulePriority[modName]) {
		case PRI_HIGH:
			modEntry = highModules.find(modName);
			delete modEntry->second;
			highModules.erase(modEntry);
			break;
		case PRI_MEDIUM_HIGH:
			modEntry = mediumHighModules.find(modName);
			delete modEntry->second;
			mediumHighModules.erase(modEntry);
			break;
		case PRI_NORMAL:
			modEntry = normalModules.find(modName);
			delete modEntry->second;
			normalModules.erase(modEntry);
			break;
		case PRI_MEDIUM_LOW:
			modEntry = mediumLowModules.find(modName);
			delete modEntry->second;
			mediumLowModules.erase(modEntry);
			break;
		case PRI_LOW:
			modEntry = lowModules.find(modName);
			delete modEntry->second;
			lowModules.erase(modEntry);
	}
	modulePriority.erase(modulePriority.find(modName));
	moduleDescriptions.erase(moduleDescriptions.find(modName));
	dlclose(moduleFiles[modName]);
	moduleFiles.erase(moduleFiles.find(modName));
	// Unload modules that require unsatisfied dependencies
	std::list<std::string> unloadList; // Use an list of modules to unload in case another unload screws up the service loops
	for (std::pair<std::string, std::list<std::string>> service : moduleServices) {
		if (service.second.empty()) {
			for (std::string modName : moduleRequires[service.first])
				unloadList.push_back(modName);
		}
	}
	for (std::string modName : unloadList)
		unloadModule(modName, true);
}

void Base::connectServer(std::string server) {
	if (servers.find(server) != servers.end())
		return; // We're already connected!
	std::map<std::string, std::map<std::string, std::string>>::iterator confIter = serverConfig.find(server);
	if (confIter == serverConfig.end())
		return;
	std::string protoType = confIter->second["protocol"];
	if (protoType == "") {
		std::cerr << "A protocol type was not specified for the server " << server << ".  Please check your configuration and try again." << std::endl;
		return;
	}
	std::unordered_map<std::string, void*>::iterator fileIter = protocolFiles.find(protoType);
	if (fileIter != protocolFiles.end()) {
		protocol_spawn_t* protoSpawn = (protocol_spawn_t*) dlsym(fileIter->second, "spawn");
		const char* spawnError = dlerror();
		if (spawnError != NULL) {
			std::cerr << "Spawn not found in protocol module for server " << server << ": " << spawnError << std::endl;
			return;
		}
		Protocol* newServer = (Protocol*) protoSpawn(server, confIter->second, workingDir, logDump, debugLevel, this);
		if (newServer->apiVersion() != 3000) {
			std::cerr << "The protocol module for server " << server << " is not compatible with this version of RoBoBo." << std::endl;
			delete newServer;
			return;
		}
		protocolTypes[protoType].insert(server);
		servers.insert(std::pair<std::string, Protocol*> (server, newServer));
		newServer->connectServer();
		return; // Success!  Return now while we're ahead (so the other stuff doesn't execute)
	}
	std::string filename = workingDir + "/modules/p_" + protoType + ".so";
	void* protoFile = dlopen(filename.c_str(), RTLD_NOW);
	const char* fileOpenError = dlerror();
	if (fileOpenError != NULL) {
		std::cerr << "The protocol module for server " << server << " could not be found: " << fileOpenError << std::endl;
		return;
	}
	protocol_spawn_t* protoSpawn = (protocol_spawn_t*) dlsym(protoFile, "spawn");
	const char* spawnError = dlerror();
	if (spawnError != NULL) {
		std::cerr << "Spawn not found in protocol module for server " << server << ": " << spawnError << std::endl;
		dlclose(protoFile);
		return;
	}
	Protocol* newServer = (Protocol*) protoSpawn(server, confIter->second, workingDir, logDump, debugLevel, this);
	if (newServer->apiVersion() != 3000) {
		std::cerr << "The protocol module for server " << server << " is not compatible with this version of RoBoBo." << std::endl;
		delete newServer;
		dlclose(protoFile);
		return;
	}
	protocolFiles.insert(std::pair<std::string, void*> (protoType, protoFile));
	protocolTypes[protoType].insert(server);
	servers.insert(std::pair<std::string, Protocol*> (server, newServer));
	newServer->connectServer();
}

void Base::disconnectServer(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	Protocol* unloadingServer = servIter->second;
	servers.erase(servIter);
	unloadingServer->disconnectServer();
	delete unloadingServer;
	std::string protoType = "";
	for (std::pair<std::string, std::set<std::string>> typeList : protocolTypes) {
		if (typeList.second.find(server) != typeList.second.end()) {
			protoType = typeList.first;
			break;
		}
	}
	std::unordered_map<std::string, std::set<std::string>>::iterator typeIter = protocolTypes.find(protoType);
	if (typeIter->second.empty()) {
		std::unordered_map<std::string, std::set<std::string>>::iterator fileIter = protocolFiles.find(protoType);
		dlclose(fileIter->second);
		protocolFiles.erase(fileIter);
		protocolTypes.erase(typeIter);
	}
}

Socket* Base::loadSocket(std::string sockettype) {
	std::unordered_map<std::string, void*>::iterator fileIter = socketFiles.find(sockettype);
	if (fileIter == socketFiles.end()) {
		std::string fileName = workingDir + "/modules/s_" + sockettype + ".so";
		void* socketFile = dlopen(fileName.c_str(), RTLD_NOW);
		const char* fileOpenError = dlerror();
		if (fileOpenError != NULL) {
			std::cerr << "The socket module s_" << sockettype << " could not be loaded: " << fileOpenError << std::endl;
			return NULL;
		}
		socket_spawn_t* socketSpawn = (socket_spawn_t*) dlsym(socketFile, "spawn");
		const char* spawnError = dlerror();
		if (spawnError != NULL) {
			std::cerr << "Spawn not found in socket module s_" << sockettype << ": " << spawnError << std::endl;
			dlclose(socketFile);
			return NULL;
		}
		Socket* newSocket = (Socket*) socketSpawn();
		if (newSocket->apiVersion() != 3000) {
			std::cerr << "The socket module s_" << sockettype << " is not compatible with this version of RoBoBo." << std::endl;
			delete newSocket;
			dlclose(socketFile);
			return NULL;
		}
		socketFiles.insert(std::pair<std::string, void*> (sockettype, socketFile));
		socketCounts[sockettype]++;
		return newSocket;
	}
	void* socketFile = socketFiles[sockettype];
	socket_spawn_t* socketSpawn = (socket_spawn_t*) dlsym(socketFile, "spawn");
	const char* spawnError = dlerror();
	if (spawnError != NULL) {
		std::cerr << "Spawn not found in socket module s_" << sockettype << ": " << spawnError << std::endl;
		return NULL;
	}
	Socket* newSocket = (Socket*) socketSpawn();
	if (newSocket->apiVersion() != 3000) {
		std::cerr << "The socket module s_" << sockettype << " is not compatible with this version of RoBoBo." << std::endl;
		delete newSocket;
		return NULL;
	}
	socketCounts[sockettype]++;
	return newSocket;
}

void Base::unloadSocket(std::string sockettype, Socket* socketptr) {
	delete socketptr;
	socketCounts[sockettype]--;
	if (socketCounts[sockettype] == 0) {
		std::unordered_map<std::string, void*>::iterator fileIter = socketFiles.find(sockettype);
		dlclose(fileIter->second);
		socketFiles.erase(fileIter);
	}
}

void Base::rehash() {
	serverConfig.clear();
	moduleConfig.clear();
	startupServers.clear();
	startupModules.clear();
	readConfiguration();
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->rehash(moduleConfig[module.first]);
		module.second->onRehash();
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->rehash(moduleConfig[module.first]);
		module.second->onRehash();
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->rehash(moduleConfig[module.first]);
		module.second->onRehash();
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->rehash(moduleConfig[module.first]);
		module.second->onRehash();
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->rehash(moduleConfig[module.first]);
		module.second->onRehash();
	}
	modHookMutex.unlock();
}

/* Make the calls asynchronous
 * The goal of this section is to make sure module functions are called one at a time, but still in order.
 * The modHook functions start a thread and return.  This allows:
 *   "nonblocking" function calls
 *   more true "in-order" receiving of function calls
 *     A message may be received on one server while it's still blocking, and then another on a different server while the first is still blocking.
 *       result: the second message gets called first
 * The threads are detached each time to allow them to complete despite the thread object going out of scope.
 * The callHooks functions are detailed below with the function definitions.
 */

void Base::modChanMsgHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	std::thread (&Base::callChanMsgHooks, this, server, client, channel, status, nick, message).detach();
}

void Base::modUserMsgHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserMsgHooks, this, server, client, nick, message).detach();
}

void Base::modChanNoticeHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	std::thread (&Base::callChanNoticeHooks, this, server, client, channel, status, nick, message).detach();
}

void Base::modUserNoticeHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserNoticeHooks, this, server, client, nick, message).detach();
}

void Base::modChanCTCPHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	std::thread (&Base::callChanCTCPHooks, this, server, client, channel, status, nick, ctcp, data).detach();
}

void Base::modUserCTCPHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	std::thread (&Base::callUserCTCPHooks, this, server, client, nick, ctcp, data).detach();
}

void Base::modChanCTCPReplyHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	std::thread (&Base::callChanCTCPReplyHooks, this, server, client, channel, status, nick, ctcp, data).detach();
}

void Base::modUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	std::thread (&Base::callUserCTCPReplyHooks, this, server, client, nick, ctcp, data).detach();
}

void Base::modChanModeHook(std::string server, std::string channel, std::string setter, bool add, std::string mode, std::string param) {
	std::thread (&Base::callChanModeHooks, this, server, channel, setter, add, mode, param).detach();
}

void Base::modUserModeHook(std::string server, std::string nick, bool add, std::string mode) {
	std::thread (&Base::callUserModeHooks, this, server, nick, add, mode).detach();
}

void Base::modUserSNOMaskHook(std::string server, std::string nick, bool add, char snomask) {
	std::thread (&Base::callUserSNOMaskHooks, this, server, nick, add, snomask).detach();
}

void Base::modChanTopicHook(std::string server, std::string channel, std::string setter, std::string topic) {
	std::thread (&Base::callChanTopicHooks, this, server, channel, setter, topic).detach();
}

void Base::modChanJoinHook(std::string server, std::string channel, std::string nick) {
	std::thread (&Base::callChanJoinHooks, this, server, channel, nick).detach();
}

void Base::modChanPartHook(std::string server, std::string channel, std::string nick, std::string reason) {
	std::thread (&Base::callChanPartHooks, this, server, channel, nick, reason).detach();
}

void Base::modChanKickHook(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	std::thread (&Base::callChanKickHooks, this, server, channel, kicker, kickee, reason).detach();
}

void Base::modUserConnectHook(std::string server, std::string nick) {
	std::thread (&Base::callUserConnectHooks, this, server, nick).detach();
}

void Base::modUserQuitHook(std::string server, std::string nick, std::string reason) {
	std::thread (&Base::callUserQuitHooks, this, server, nick, reason).detach();
}

void Base::modUserNickHook(std::string server, std::string oldNick, std::string newNick) {
	std::thread (&Base::callUserNickHooks, this, server, oldNick, newNick).detach();
}

void Base::modNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {
	std::thread (&Base::callNumericHooks, this, server, client, numeric, data).detach();
}

void Base::modOperHook(std::string server, std::string nick, std::string operType) {
	std::thread (&Base::callOperHooks, this, server, nick, operType).detach();
}

void Base::modSNoticeHook(std::string server, char snotype, std::string message) {
	std::thread (&Base::callSNoticeHooks, this, server, snotype, message).detach();
}

void Base::modMetadataHook(std::string server, std::string target, std::string dataKey, std::string dataValue) {
	std::thread (&Base::callMetadataHooks, this, server, target, dataKey, dataValue).detach();
}

void Base::modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {
	std::thread (&Base::callXLineAddHooks, this, server, lineType, mask, setter, expiry, reason).detach();
}

void Base::modXLineRemoveHook(std::string server, std::string lineType, std::string mask) {
	std::thread (&Base::callXLineRemoveHooks, this, server, lineType, mask).detach();
}

void Base::modServerConnectHook(std::string server, std::string newServerName) {
	std::thread (&Base::callServerConnectHooks, this, server, newServerName).detach();
}

void Base::modServerQuitHook(std::string server, std::string quitServerName, std::string reason) {
	std::thread (&Base::callServerQuitHooks, this, server, quitServerName, reason).detach();
}

void Base::modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens) {
	std::thread (&Base::callOtherDataHooks, this, server, client, lineTokens).detach();
}

void Base::modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	std::thread (&Base::callChanMsgOutHooks, this, server, client, channel, status, message).detach();
}

void Base::modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	std::thread (&Base::callChanMsgSendHooks, this, server, client, channel, status, message).detach();
}

void Base::modUserMsgOutHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserMsgOutHooks, this, server, client, nick, message).detach();
}

void Base::modUserMsgSendHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserMsgSendHooks, this, server, client, nick, message).detach();
}

void Base::modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	std::thread (&Base::callChanNoticeOutHooks, this, server, client, channel, status, message).detach();
}

void Base::modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	std::thread (&Base::callChanNoticeSendHooks, this, server, client, channel, status, message).detach();
}

void Base::modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserNoticeOutHooks, this, server, client, nick, message).detach();
}

void Base::modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message) {
	std::thread (&Base::callUserNoticeSendHooks, this, server, client, nick, message).detach();
}

void Base::modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	std::thread (&Base::callChanCTCPOutHooks, this, server, client, channel, status, ctcp, params).detach();
}

void Base::modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	std::thread (&Base::callChanCTCPSendHooks, this, server, client, channel, status, ctcp, params).detach();
}

void Base::modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	std::thread (&Base::callUserCTCPOutHooks, this, server, client, nick, ctcp, params).detach();
}

void Base::modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	std::thread (&Base::callUserCTCPSendHooks, this, server, client, nick, ctcp, params).detach();
}

void Base::modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	std::thread (&Base::callChanCTCPReplyOutHooks, this, server, client, channel, status, ctcp, params).detach();
}

void Base::modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	std::thread (&Base::callChanCTCPReplySendHooks, this, server, client, channel, status, ctcp, params).detach();
}

void Base::modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	std::thread (&Base::callUserCTCPReplyOutHooks, this, server, client, nick, ctcp, params).detach();
}

void Base::modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	std::thread (&Base::callUserCTCPReplySendHooks, this, server, client, nick, ctcp, params).detach();
}

void Base::sendPrivMsg(std::string server, std::string client, std::string target, std::string message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendPrivMsg(client, target, message);
}

void Base::sendNotice(std::string server, std::string client, std::string target, std::string message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendNotice(client, target, message);
}

void Base::sendCTCP(std::string server, std::string client, std::string target, std::string ctcp, std::string params) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendCTCP(client, target, ctcp, params);
}

void Base::sendCTCPReply(std::string server, std::string client, std::string target, std::string ctcp, std::string params) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendCTCPReply(client, target, ctcp, params);
}

void Base::setMode(std::string server, std::string client, std::string target, std::list<std::string> setModes, std::list<std::string> delModes) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setMode(client, target, setModes, delModes);
}

void Base::setSNOMask(std::string server, std::string client, char snomask, bool add) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setSNOMask(client, snomask, add);
}

void Base::setChanTopic(std::string server, std::string client, std::string channel, std::string topic) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setChanTopic(client, channel, topic);
}

void Base::joinChannel(std::string server, std::string client, std::string channel, std::string key) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->joinChannel(client, channel, key);
}

void Base::partChannel(std::string server, std::string client, std::string channel, std::string reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->partChannel(client, channel, reason);
}

void Base::changeNick(std::string server, std::string client, std::string newNick) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeNick(client, newNick);
}

void Base::kickUser(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->kickUser(client, channel, nick, reason);
}

std::string Base::addClient(std::string server, std::string nick, std::string ident, std::string host, std::string gecos) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->addClient(nick, ident, host, gecos);
}

void Base::removeClient(std::string server, std::string client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->removeClient(client);
}

void Base::oper(std::string server, std::string client, std::string username, std::string password) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->oper(client, username, password);
}

void Base::sendSNotice(std::string server, char snomask, std::string message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendSNotice(snomask, message);
}

void Base::setMetadata(std::string server, std::string target, std::string key, std::string value) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setMetadata(target, key, value);
}

void Base::setXLine(std::string server, std::string client, std::string linetype, std::string mask, time_t duration, std::string reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setXLine(client, linetype, mask, duration, reason);
}

void Base::delXLine(std::string server, std::string client, std::string linetype, std::string mask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->delXLine(client, linetype, mask);
}

void Base::sendOtherData(std::string server, std::string client, std::string line) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendOtherData(client, line);
}

std::list<std::string> Base::activeServers() {
	std::list<std::string> serverList;
	for (std::pair<std::string, Protocol*> server : servers)
		serverList.push_back(server.first);
	return serverList;
}

std::list<std::string> Base::connectedServers() {
	std::list<std::string> serverList;
	for (std::pair<std::string, Protocol*> server : servers) {
		if (server.second->isConnected())
			serverList.push_back(server.first);
	}
	return serverList;
}

bool Base::serverIsClient(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->isClient();
}

std::string Base::serverType(std::string server) {
	if (servers.find(server) == servers.end())
		return "";
	for (std::pair<std::string, std::set<std::string>> providers : protocolTypes) {
		if (providers.second.find(server) != providers.second.end())
			return providers.first;
	}
	return "";
}

std::list<std::string> Base::serverListModes(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->listModes();
}

std::list<std::string> Base::serverParamModes(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->paramModes();
}

std::list<std::string> Base::serverModes(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->modes();
}

std::list<std::pair<std::string, char>> Base::serverStatuses(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::pair<std::string, char>> ();
	return servIter->second->statuses();
}

std::list<std::string> Base::channels(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->channels();
}

std::list<std::string> Base::inChannels(std::string server, std::string client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->inChannels(client);
}

std::list<std::string> Base::channelUsers(std::string server, std::string channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->channelUsers(channel);
}

bool Base::userInChannel(std::string server, std::string channel, std::string user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userInChannel(channel, user);
}

std::string Base::channelTopic(std::string server, std::string channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->channelTopic(channel);
}

std::list<std::string> Base::channelModes(std::string server, std::string channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->channelModes(channel);
}

bool Base::channelHasMode(std::string server, std::string channel, std::string mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->channelHasMode(channel, mode);
}

std::string Base::modeParam(std::string server, std::string channel, std::string mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->modeParam(channel, mode);
}

std::list<std::string> Base::channelListMode(std::string server, std::string channel, std::string mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->channelListMode(channel, mode);
}

bool Base::channelListHasEntry(std::string server, std::string channel, std::string listMode, std::string entry) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->channelListHasEntry(channel, listMode, entry);
}

std::pair<std::string, char> Base::userStatus(std::string server, std::string channel, std::string user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->userStatus(channel, user);
}

std::pair<std::string, char> Base::compareStatus(std::string server, std::string status0, std::string status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::pair<std::string, char> Base::compareStatus(std::string server, std::string status0, char status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::pair<std::string, char> Base::compareStatus(std::string server, char status0, std::string status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::pair<std::string, char> Base::compareStatus(std::string server, char status0, char status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

bool Base::userHasStatus(std::string server, std::string channel, std::string user, std::string status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatus(channel, user, status);
}

bool Base::userHasStatus(std::string server, std::string channel, std::string user, char status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatus(channel, user, status);
}

bool Base::userHasStatusOrGreater(std::string server, std::string channel, std::string user, std::string status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatusOrGreater(channel, user, status);
}

bool Base::userHasStatusOrGreater(std::string server, std::string channel, std::string user, char status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatusOrGreater(channel, user, status);
}

std::list<std::string> Base::clients(std::string server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->clients();
}

std::list<std::string> Base::userModes(std::string server, std::string client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->userModes(client);
}

bool Base::hasUserMode(std::string server, std::string client, std::string mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->hasUserMode(client, mode);
}

std::list<char> Base::snomasks(std::string server, std::string client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<char> ();
	return servIter->second->snomasks(client);
}

bool Base::hasSNOMask(std::string server, std::string client, char snomask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->hasSNOMask(client, snomask);
}

std::list<std::string> Base::userChannels(std::string server, std::string nick) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->userChannels(nick);
}

std::list<std::string> Base::moduleList() {
	std::list<std::string> modList;
	for (std::pair<std::string, Module*> module : highModules)
		modList.push_back(module.first);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		modList.push_back(module.first);
	for (std::pair<std::string, Module*> module : normalModules)
		modList.push_back(module.first);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		modList.push_back(module.first);
	for (std::pair<std::string, Module*> module : lowModules)
		modList.push_back(module.first);
	return modList;
}

std::list<std::string> Base::providedServices() {
	std::list<std::string> services;
	for (std::pair<std::string, std::list<std::string>> service : moduleServices) {
		if (!service.second.empty())
			services.push_back(service.first);
	}
	return services;
}

bool Base::serviceIsProvided(std::string service) {
	return !moduleServices[service].empty();
}

std::list<std::string> Base::serviceProviders(std::string service) {
	return moduleServices[service];
}

std::list<std::string> Base::serviceUsers(std::string service) {
	return moduleSupports[service];
}

void Base::refreshServices(std::string module) {
	std::map<std::string, Module*>::iterator modIter = highModules.find(module);
	if (modIter == highModules.end()) {
		modIter = mediumHighModules.find(module);
		if (modIter == mediumHighModules.end()) {
			modIter = normalModules.find(module);
			if (modIter == normalModules.end()) {
				modIter = mediumLowModules.find(module);
				if (modIter == mediumLowModules.end()) {
					modIter = lowModules.find(module);
					if (modIter == lowModules.end())
						return; // Some sort of serious error occurred!
				}
			}
		}
	}
	for (std::pair<std::string, std::list<std::string>> service : moduleServices)
		service.second.remove(module);
	for (std::pair<std::string, std::list<std::string>> service : moduleRequires)
		service.second.remove(module);
	for (std::pair<std::string, std::list<std::string>> service : moduleSupports)
		service.second.remove(module);
	std::list<std::string> provides = modIter->second->provides();
	for (std::string service : provides)
		moduleServices[service].push_back(module);
	std::list<std::string> requires = modIter->second->requires();
	for (std::string service : requires) {
		moduleRequires[service].push_back(module);
		moduleSupports[service].push_back(module);
	}
	std::list<std::string> supports = modIter->second->supports();
	for (std::string service : supports)
		moduleSupports[service].push_back(module);
}

/* The callHooks functions are called as part of a thread
 * This prevents unnecessary blocking of the protocol modules as they continue to communicate with the server
 *  and also tries to keep in-order calling of module hooks
 */

void Base::callChanMsgHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onChanMsg(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onChanMsg(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onChanMsg(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onChanMsg(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onChanMsg(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callUserMsgHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onUserMsg(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onUserMsg(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onUserMsg(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onUserMsg(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onUserMsg(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callChanNoticeHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onChanNotice(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onChanNotice(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onChanNotice(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onChanNotice(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onChanNotice(server, client, channel, status, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callUserNoticeHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onUserNotice(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onUserNotice(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onUserNotice(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onUserNotice(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onUserNotice(server, client, nick, message) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callChanCTCPHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onChanCTCP(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onChanCTCP(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onChanCTCP(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onChanCTCP(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onChanCTCP(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callUserCTCPHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onUserCTCP(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onUserCTCP(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onUserCTCP(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onUserCTCP(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onUserCTCP(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callChanCTCPReplyHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onChanCTCPReply(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onChanCTCPReply(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onChanCTCPReply(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onChanCTCPReply(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onChanCTCPReply(server, client, channel, status, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callUserCTCPReplyHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		if (module.second->onUserCTCPReply(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		if (module.second->onUserCTCPReply(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		if (module.second->onUserCTCPReply(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		if (module.second->onUserCTCPReply(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		if (module.second->onUserCTCPReply(server, client, nick, ctcp, data) == MSG_IGNORE) {
			modHookMutex.unlock();
			return;
		}
	}
	modHookMutex.unlock();
}

void Base::callChanModeHooks(std::string server, std::string channel, std::string setter, bool add, std::string mode, std::string param) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanMode(server, channel, setter, add, mode, param);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanMode(server, channel, setter, add, mode, param);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanMode(server, channel, setter, add, mode, param);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanMode(server, channel, setter, add, mode, param);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanMode(server, channel, setter, add, mode, param);
	modHookMutex.unlock();
}

void Base::callUserModeHooks(std::string server, std::string nick, bool add, std::string mode) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserMode(server, nick, add, mode);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserMode(server, nick, add, mode);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserMode(server, nick, add, mode);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserMode(server, nick, add, mode);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserMode(server, nick, add, mode);
	modHookMutex.unlock();
}

void Base::callUserSNOMaskHooks(std::string server, std::string nick, bool add, char snomask) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserSNOMask(server, nick, add, snomask);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserSNOMask(server, nick, add, snomask);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserSNOMask(server, nick, add, snomask);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserSNOMask(server, nick, add, snomask);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserSNOMask(server, nick, add, snomask);
	modHookMutex.unlock();
}

void Base::callChanTopicHooks(std::string server, std::string channel, std::string setter, std::string topic) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanTopic(server, channel, setter, topic);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanTopic(server, channel, setter, topic);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanTopic(server, channel, setter, topic);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanTopic(server, channel, setter, topic);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanTopic(server, channel, setter, topic);
	modHookMutex.unlock();
}

void Base::callChanJoinHooks(std::string server, std::string channel, std::string nick) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanJoin(server, channel, nick);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanJoin(server, channel, nick);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanJoin(server, channel, nick);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanJoin(server, channel, nick);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanJoin(server, channel, nick);
	modHookMutex.unlock();
}

void Base::callChanPartHooks(std::string server, std::string channel, std::string nick, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanPart(server, channel, nick, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanPart(server, channel, nick, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanPart(server, channel, nick, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanPart(server, channel, nick, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanPart(server, channel, nick, reason);
	modHookMutex.unlock();
}

void Base::callChanKickHooks(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanKick(server, channel, kicker, kickee, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanKick(server, channel, kicker, kickee, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanKick(server, channel, kicker, kickee, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanKick(server, channel, kicker, kickee, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanKick(server, channel, kicker, kickee, reason);
	modHookMutex.unlock();
}

void Base::callUserConnectHooks(std::string server, std::string nick) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserConnect(server, nick);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserConnect(server, nick);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserConnect(server, nick);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserConnect(server, nick);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserConnect(server, nick);
	modHookMutex.unlock();
}

void Base::callUserQuitHooks(std::string server, std::string nick, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserQuit(server, nick, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserQuit(server, nick, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserQuit(server, nick, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserQuit(server, nick, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserQuit(server, nick, reason);
	modHookMutex.unlock();
}

void Base::callUserNickHooks(std::string server, std::string oldNick, std::string newNick) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserNick(server, oldNick, newNick);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserNick(server, oldNick, newNick);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserNick(server, oldNick, newNick);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserNick(server, oldNick, newNick);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserNick(server, oldNick, newNick);
	modHookMutex.unlock();
}

void Base::callNumericHooks(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onNumeric(server, client, numeric, data);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onNumeric(server, client, numeric, data);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onNumeric(server, client, numeric, data);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onNumeric(server, client, numeric, data);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onNumeric(server, client, numeric, data);
	modHookMutex.unlock();
}

void Base::callOperHooks(std::string server, std::string nick, std::string operType) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onOper(server, nick, operType);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onOper(server, nick, operType);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onOper(server, nick, operType);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onOper(server, nick, operType);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onOper(server, nick, operType);
	modHookMutex.unlock();
}

void Base::callSNoticeHooks(std::string server, char snotype, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onSNotice(server, snotype, message);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onSNotice(server, snotype, message);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onSNotice(server, snotype, message);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onSNotice(server, snotype, message);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onSNotice(server, snotype, message);
	modHookMutex.unlock();
}

void Base::callMetadataHooks(std::string server, std::string target, std::string dataKey, std::string dataValue) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onMetadata(server, target, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onMetadata(server, target, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onMetadata(server, target, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onMetadata(server, target, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onMetadata(server, target, dataKey, dataValue);
	modHookMutex.unlock();
}

void Base::callXLineAddHooks(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onXLineAdd(server, lineType, mask, setter, expiry, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onXLineAdd(server, lineType, mask, setter, expiry, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onXLineAdd(server, lineType, mask, setter, expiry, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onXLineAdd(server, lineType, mask, setter, expiry, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onXLineAdd(server, lineType, mask, setter, expiry, reason);
	modHookMutex.unlock();
}

void Base::callXLineRemoveHooks(std::string server, std::string lineType, std::string mask) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onXLineRemove(server, lineType, mask);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onXLineRemove(server, lineType, mask);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onXLineRemove(server, lineType, mask);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onXLineRemove(server, lineType, mask);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onXLineRemove(server, lineType, mask);
	modHookMutex.unlock();
}

void Base::callServerConnectHooks(std::string server, std::string newServerName) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onServerConnect(server, newServerName);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onServerConnect(server, newServerName);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onServerConnect(server, newServerName);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onServerConnect(server, newServerName);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onServerConnect(server, newServerName);
	modHookMutex.unlock();
}

void Base::callServerQuitHooks(std::string server, std::string quitServerName, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onServerQuit(server, quitServerName, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onServerQuit(server, quitServerName, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onServerQuit(server, quitServerName, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onServerQuit(server, quitServerName, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onServerQuit(server, quitServerName, reason);
	modHookMutex.unlock();
}

void Base::callOtherDataHooks(std::string server, std::string client, std::vector<std::string> lineTokens) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onOtherData(server, client, lineTokens);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onOtherData(server, client, lineTokens);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onOtherData(server, client, lineTokens);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onOtherData(server, client, lineTokens);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onOtherData(server, client, lineTokens);
	modHookMutex.unlock();
}

void Base::callChanMsgOutHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	servers.find(server)->second->processedChanMsg(client, channel, status, message);
	modHookMutex.unlock();
}

void Base::callChanMsgSendHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanMsgSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanMsgSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanMsgSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanMsgSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanMsgSend(server, client, channel, status, message);
	modHookMutex.unlock();
}

void Base::callUserMsgOutHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	servers.find(server)->second->processedUserMsg(client, nick, message);
	modHookMutex.unlock();
}

void Base::callUserMsgSendHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserMsgSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserMsgSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserMsgSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserMsgSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserMsgSend(server, client, nick, message);
	modHookMutex.unlock();
}

void Base::callChanNoticeOutHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	servers.find(server)->second->processedChanNotice(client, channel, status, message);
	modHookMutex.unlock();
}

void Base::callChanNoticeSendHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanNoticeSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanNoticeSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanNoticeSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanNoticeSend(server, client, channel, status, message);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanNoticeSend(server, client, channel, status, message);
	modHookMutex.unlock();
}

void Base::callUserNoticeOutHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "") {
			modHookMutex.unlock();
			return;
		}
	}
	servers.find(server)->second->processedUserNotice(client, nick, message);
	modHookMutex.unlock();
}

void Base::callUserNoticeSendHooks(std::string server, std::string client, std::string nick, std::string message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserNoticeSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserNoticeSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserNoticeSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserNoticeSend(server, client, nick, message);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserNoticeSend(server, client, nick, message);
	modHookMutex.unlock();
}

void Base::callChanCTCPOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	if (params == "")
		servers.find(server)->second->processedChanCTCP(client, channel, status, ctcp);
	else
		servers.find(server)->second->processedChanCTCP(client, channel, status, ctcp + " " + params);
	modHookMutex.unlock();
}

void Base::callChanCTCPSendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanCTCPSend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanCTCPSend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanCTCPSend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanCTCPSend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanCTCPSend(server, client, channel, status, ctcp, params);
	modHookMutex.unlock();
}

void Base::callUserCTCPOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	if (params == "")
		servers.find(server)->second->processedUserCTCP(client, nick, ctcp);
	else
		servers.find(server)->second->processedUserCTCP(client, nick, ctcp + " " + params);
	modHookMutex.unlock();
}

void Base::callUserCTCPSendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserCTCPSend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserCTCPSend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserCTCPSend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserCTCPSend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserCTCPSend(server, client, nick, ctcp, params);
	modHookMutex.unlock();
}

void Base::callChanCTCPReplyOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	if (params == "")
		servers.find(server)->second->processedChanCTCPReply(client, channel, status, ctcp);
	else
		servers.find(server)->second->processedChanCTCPReply(client, channel, status, ctcp + " " + params);
	modHookMutex.unlock();
}

void Base::callChanCTCPReplySendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanCTCPReplySend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanCTCPReplySend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanCTCPReplySend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanCTCPReplySend(server, client, channel, status, ctcp, params);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanCTCPReplySend(server, client, channel, status, ctcp, params);
	modHookMutex.unlock();
}

void Base::callUserCTCPReplyOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "") {
			modHookMutex.unlock();
			return;
		}
	}
	if (params == "")
		servers.find(server)->second->processedUserCTCPReply(client, nick, ctcp);
	else
		servers.find(server)->second->processedUserCTCPReply(client, nick, ctcp + " " + params);
	modHookMutex.unlock();
}

void Base::callUserCTCPReplySendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserCTCPReplySend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserCTCPReplySend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserCTCPReplySend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserCTCPReplySend(server, client, nick, ctcp, params);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserCTCPReplySend(server, client, nick, ctcp, params);
	modHookMutex.unlock();
}