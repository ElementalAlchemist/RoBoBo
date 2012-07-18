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
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(60));
		std::list<std::string> removeServers;
		for (std::pair<std::string, Protocol*> server : servers) {
			if (server.second->deadServer())
				removeServers.push_back(server.first);
		}
		for (std::string server : removeServers)
			disconnectServer(server);
		if (servers.empty()) {
			bool keepAlive = false;
			for (std::pair<std::string, Module*> module : highModules) {
				if (module.second->forceKeepAlive()) {
					keepAlive = true;
					break;
				}
			}
			if (!keepAlive) {
				for (std::pair<std::string, Module*> module : mediumHighModules) {
					if (module.second->forceKeepAlive()) {
						keepAlive = true;
						break;
					}
				}
				if (!keepAlive) {
					for (std::pair<std::string, Module*> module : normalModules) {
						if (module.second->forceKeepAlive()) {
							keepAlive = true;
							break;
						}
					}
					if (!keepAlive) {
						for (std::pair<std::string, Module*> module : mediumLowModules) {
							if (module.second->forceKeepAlive()) {
								keepAlive = true;
								break;
							}
						}
						if (!keepAlive) {
							for (std::pair<std::string, Module*> module : lowModules) {
								if (module.second->forceKeepAlive()) {
									keepAlive = true;
									break;
								}
							}
						}
					}
				}
			}
			if (!keepAlive)
				return;
		}
	}
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
	modulePriority.insert(std::pair<std::string, Priority> (modName, newModule->priority()));
	switch (newModule->priority()) {
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
			MutexManager hookManage (&modHookMutex);
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
		MutexManager hookManage (&modHookMutex);
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
	MutexManager hookManage(&modHookMutex);
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onConnect(server);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onConnect(server);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onConnect(server);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onConnect(server);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onConnect(server);
}

void Base::disconnectServer(std::string server, std::string reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	Protocol* unloadingServer = servIter->second;
	servers.erase(servIter);
	unloadingServer->disconnectServer(reason);
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
		std::unordered_map<std::string, void*>::iterator fileIter = protocolFiles.find(protoType);
		dlclose(fileIter->second);
		protocolFiles.erase(fileIter);
		protocolTypes.erase(typeIter);
	}
	MutexManager hookManage (&modHookMutex);
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onDisconnect(server);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onDisconnect(server);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onDisconnect(server);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onDisconnect(server);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onDisconnect(server);
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
	std::shared_ptr<Socket> newSocket ((Socket*) socketSpawn(), std::bind(&Base::unloadSocket, this, sockettype, std::placeholders::_1));
	if (newSocket->apiVersion() != 3000) {
		std::cerr << "The socket module s_" << sockettype << " is not compatible with this version of RoBoBo." << std::endl;
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
	/* Because rehash is almost always initiated from a module (and this version always is), it's a bad idea to specify the mutex
	 * here (WILL result in deadlock).  Instead, have modules expect config data (and any other data changed in onRehash) to change
	 * during the call to rehash.  The use of the hook mutex in the other functions guarantees that nothing else is called during
	 * this time, so only the module calling rehash can expect things to change suddenly, and only during the (blocking) call to rehash.
	 */
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
}

void Base::sigRehash() {
	// This function is pretty much the same as rehash() above, except it activates the module hook mutex.
	// This prevents the rehash signal from changing things in the middle of other calls
	serverConfig.clear();
	moduleConfig.clear();
	startupServers.clear();
	startupModules.clear();
	readConfiguration();
	// This mutex manager automatically releases the mutex on function exit
	MutexManager hookManage (&modHookMutex);
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
}

void Base::endDebug() {
	for (std::pair<std::string, Protocol*> server : servers)
		server.second->endDebug();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->endDebug();
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->endDebug();
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->endDebug();
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->endDebug();
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->endDebug();
}

void Base::sendPrivMsg(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendPrivMsg(client, target, message);
}

void Base::sendNotice(const std::string& server, const std::string& client, const std::string& target, const std::string& message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendNotice(client, target, message);
}

void Base::sendCTCP(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendCTCP(client, target, ctcp, params);
}

void Base::sendCTCPReply(const std::string& server, const std::string& client, const std::string& target, const std::string& ctcp, const std::string& params) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendCTCPReply(client, target, ctcp, params);
}

void Base::setMode(const std::string& server, const std::string& client, const std::string& target, const std::list<std::string>& setModes, const std::list<std::string>& remModes) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setMode(client, target, setModes, remModes);
}

void Base::setSNOmask(const std::string& server, const std::string& client, bool add, char snomask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setSNOmask(client, add, snomask);
}

void Base::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->joinChan(client, channel, key);
}

void Base::partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->partChan(client, channel, reason);
}

void Base::kickUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user, const std::string& reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->kickUser(client, channel, user, reason);
}

std::string Base::addClient(const std::string& server, std::string& nick, std::string& ident, std::string& host, std::string& gecos) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->addClient(nick, ident, host, gecos);
}

void Base::removeClient(const std::string& server, const std::string& client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->removeClient(client);
}

void Base::setTopic(const std::string& server, const std::string& client, const std::string& channel, const std::string& topic) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setTopic(client, channel, topic);
}

void Base::inviteUser(const std::string& server, const std::string& client, const std::string& channel, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->inviteUser(client, channel, user);
}

void Base::knockOnChannel(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->knockOnChannel(client, channel, reason);
}

void Base::changeNick(const std::string& server, const std::string& user, const std::string& newNick) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeNick(user, newNick);
}

void Base::sendPing(const std::string& server, const std::string& remoteServer) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendPing(remoteServer);
}

void Base::operUp(const std::string& server, const std::string& client, const std::string& usernameOrType, const std::string& password) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->operUp(client, usernameOrType, password);
}

void Base::sendServerNotice(const std::string& server, char snomask, const std::string& message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendServerNotice(snomask, message);
}

void Base::setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setMetadata(target, key, value);
}

void Base::setXLine(const std::string& server, const std::string& client, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->setXLine(client, lineType, mask, duration, reason);
}

void Base::remXLine(const std::string& server, const std::string& client, const std::stirng& lineType, const std::string& mask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->remXLine(client, lineType, mask);
}

void Base::changeIdent(const std::string& server, const std::string& user, const std::string& newIdent) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeIdent(user, newIdent);
}

void Base::changeHost(const std::string& server, const std::string& user, const std::string& newHost) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeHost(user, newHost);
}

void Base::changeGecos(const std::string& server, const std::stirng& user, const std::string& newGecos) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->changeGecos(user, newGecos);
}

void Base::sendWallops(const std::string& server, const std::string& client, const std::string& message) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendWallops(client, message);
}

void Base::sendOtherData(const std::string& server, const std::string& client, const std::string& line) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return;
	servIter->second->sendOtherData(client, line);
}

std::list<std::string> Base::activeServers() {
	std::list<std::string> serverNames;
	for (std::pair<std::string, Protocol*> server : servers)
		serverNames.push_back(server.first);
	return serverNames;
}

std::list<std::string> Base::connectedServers() {
	std::list<std::string> serverNames;
	for (std::pair<std::string, Protocol*> server : servers) {
		if (server.second->isConnected())
			serverNames.push_back(server.first);
	}
	return serverNames;
}

std::list<std::string> Base::loadedModules() {
	std::list<std::string> moduleNames;
	for (std::pair<std::string, Module*> module : highModules)
		moduleNames.push_back(module.first);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		moduleNames.push_back(module.first);
	for (std::pair<std::string, Module*> module : normalModules)
		moduleNames.push_back(module.first);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		moduleNames.push_back(module.first);
	for (std::pair<std::string, Module*> module : lowModules)
		moduleNames.push_back(module.first);
	return moduleNames;
}

std::string Base::serverType(const std::string& server) {
	if (servers.find(server) == servers.end())
		return "";
	for (std::pair<std::string, std::set<std::string>> protoType : protocolTypes) {
		if (protoType.second.find(server) != protoType.second.end())
			return protoType.first;
	}
	return ""; // couldn't find it :O
}

bool Base::serverIsClient(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->isClient();
}

std::set<std::string> Base::providedServices() {
	std::set<std::string> services;
	for (std::pair<std::string, std::list<std::string>> service : moduleServices)
		services.insert(service.first);
	return services;
}

bool Base::serviceIsProvided(const std::string& service) {
	return (moduleServices.find(service) != moduleServices.end());
}

std::list<std::string> Base::serviceProviders(const std::string& service) {
	std::map<std::string, std::list<std::string>>::iterator serviceIter = moduleServices.find(service);
	if (serviceIter == moduleServices.end())
		return std::list<std::string> ();
	return serviceIter->second;
}

std::list<std::string> Base::serviceUsers(const std::string& service) {
	std::list<std::string> users;
	std::map<std::string, std::list<std::string>>::iterator reqIter = moduleRequires.find(service);
	if (reqIter != moduleRequires.end())
		users = reqIter->second;
	std::map<std::string, std::list<std::string>>::iterator supIter = moduleSupports.find(service);
	if (supIter != moduleSupports.end()) {
		for (std::string module : supIter->second)
			users.push_back(module);
	}
	return users;
}

void Base::refreshServices(const std::string& modName, Module* modptr) {
	for (std::pair<std::string, std::list<std::string>> provIter : moduleServices)
		provIter->second.remove(modName);
	for (std::pair<std::string, std::list<std::string>> reqIter : moduleRequires)
		reqIter->second.remove(modName);
	for (std::pair<std::string, std::list<std::string>> supIter : moduleSupports)
		supIter->second.remove(modName);
	std::list<std::string> provided = modptr->provides();
	for (std::string service : provided)
		moduleServices[service].push_back(modName);
	std::list<std::string> required = modptr->requires();
	for (std::string service : required)
		moduleRequires[service].push_back(modName);
	std::list<std::string> supported = modptr->supports();
	for (std::string service : supported)
		moduleSupports[service].push_back(modName);
}

std::list<std::string> Base::networkServerList(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->networkServerList();
}

std::list<std::string> Base::xLineTypes(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->xLineTypes();
}

std::list<std::string> Base::xLineList(const std::string& server, const std::string& lineType) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->xLineList(lineType);
}

time_t Base::xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->xLineExpiry(lineType, mask);
}

std::string Base::xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->xLineReason(lineType, mask);
}

std::list<std::string> Base::chanListModes(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->chanListModes();
}

std::list<std::string> Base::chanParamModes(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->chanParamModes();
}

std::list<std::string> Base::chanNoParamModes(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->chanNoParamModes();
}

std::list<std::pair<std::string, char>> Base::chanPrefixes(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::pair<std::string, char>> ();
	return servIter->second->chanPrefixes();
}

std::pair<std::string, char> Base::compareStatus(const std::string& server, const std::string& status0, const std::string& status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::pair<std::string, char> Base::compareStatus(const std::string& server, const std::string& status0, char status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::pair<std::string, char> Base::compareStatus(const std::string& server, char status0, char status1) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->compareStatus(status0, status1);
}

std::string Base::chanTopic(const std::string& server, const std::string& channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->chanTopic(channel);
}

time_t Base::chanTimestamp(const std::string& server, const std::string& channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return 0;
	return servIter->second->chanTimestamp(channel);
}

std::set<std::string> Base::chanUsers(const std::string& server, const std::string& channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::set<std::string> ();
	return servIter->second->chanUsers(channel);
}

bool Base::userInChan(const std::string& server, const std::string& channel, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userInChan(channel, user);
}

std::pair<std::string, char> Base::userStatus(const std::string& server, const std::string& channel, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::pair<std::string, char> ("", ' ');
	return servIter->second->userStatus(channel, user);
}

bool Base::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatus(channel, user, status);
}

bool Base::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatus(channel, user, status);
}

bool Base::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatusOrGreater(channel, user, status);
}

bool Base::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->userHasStatusOrGreater(channel, user, status);
}

std::list<std::string> Base::chanModes(const std::string& server, const std::string& channel) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->chanModes(channel);
}

std::list<std::string> Base::chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->chanListModeList(channel, listMode);
}

bool Base::chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return false;
	return servIter->second->chanHasMode(channel, mode);
}

std::string Base::chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->chanModeParam(channel, mode);
}

std::list<std::string> Base::clientList(const std::string& server) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->clientList();
}

std::string Base::clientNick(const std::string& server, const std::string& client) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->clientNick(client);
}

std::string Base::userIdent(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userIdent(user);
}

std::string Base::userHost(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userHost(user);
}

std::string Base::userGecos(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return "";
	return servIter->second->userGecos(user);
}

std::list<std::string> Base::userModes(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->userModes(user);
}

std::list<char> Base::userSNOmasks(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<char> ();
	return servIter->second->userSNOmasks(user);
}

std::list<std::string> Base::userChans(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return std::list<std::string> ();
	return servIter->second->userChans(user);
}

time_t Base::userTimestamp(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return 0;
	return servIter->second->userTimestamp(user);
}

time_t Base::userNickTimestamp(const std::string& server, const std::string& user) {
	std::map<std::string, Protocol*>::iterator servIter = servers.find(server);
	if (servIter == servers.end())
		return 0;
	return servIter->second->userNickTimestamp(user);
}

void Base::callChanMsgHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	
}

void Base::callUserMsgHooks(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::callChanNoticeHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	
}

void Base::callUserNoticeHooks(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::callChanCTCPHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params) {
	
}

void Base::callUserCTCPHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	
}

void Base::callChanCTCPReplyHooks(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string params) {
	
}

void Base::callUserCTCPReplyHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	
}

void Base::callChanModeHooks(std::string server, std::string channel, bool add, std::string mode) {
	
}

void Base::callUserModeHooks(std::string server, std::string nick, bool add, std::string mode) {
	
}

void Base::callUserSNOmaskHooks(std::string server, std::string nick, bool add, char snomask) {
	
}

void Base::callChanJoinHooks(std::string server, std::string channel, std::string user) {
	
}

void Base::callChanPartHooks(std::string server, std::string channel, std::string user, std::string reason) {
	
}

void Base::callChanKickHooks(std::string server, std::string channel, std::string kicker, std::string kickee, std::string reason) {
	
}

void Base::callChanTopicHooks(std::string server, std::string channel, std::string nick, std::string topic) {
	
}

void Base::callChanInviteHooks(std::string server, std::string channel, std::string inviter, std::string invitee) {
	
}

void Base::callChanKnockHooks(std::string server, std::string channel, std::string nick, std::string reason) {
	
}

void Base::callUserNickHooks(std::string server, std::string oldNick, std::string newNick) {
	
}

void Base::callServerPingHooks(std::string server, std::string sourceServer) {
	
}

void Base::callServerPongHooks(std::string server, std::string sourceServer) {
	
}

void Base::callNumericHooks(std::string server, std::string numeric, std::vector<std::string> data) {
	
}

void Base::callUserOperHooks(std::string server, std::string nick, std::string operType) {
	
}

void Base::callServerNoticeHooks(std::string server, char snomask, std::string message) {
	
}

void Base::callMetadataHooks(std::string server, std::string target, std::string key, std::string value) {
	
}

void Base::callXLineAddHooks(std::string server, std::string lineType, std::string mask) {
	
}

void Base::callXLineRemoveHooks(std::string server, std::string lineType, std::string mask) {
	
}

void Base::callUserConnectHooks(std::string server, std::string nick) {
	
}

void Base::callUserQuitHooks(std::string server, std::string nick, std::string reason) {
	
}

void Base::callUserIdentChangeHooks(std::string server, std::string nick, std::string oldIdent, std::string newIdent) {
	
}

void Base::callUserHostChangeHooks(std::string server, std::string nick, std::string oldHost, std::string newHost) {
	
}

void Base::callUserGecosChangeHooks(std::string server, std::string nick, std::string oldGecos, std::string newGecos) {
	
}

void Base::callServerWallopsHooks(std::string server, std::string nick, std::string message) {
	
}

void Base::callServerConnectHooks(std::string server, std::string serverName) {
	
}

void Base::callServerDisconnectHooks(std::string server, std::string serverName, std::string reason) {
	
}

void Base::callServerCapHooks(std::string server, std::string subcmd, std::string list) {
	
}

void Base::callServerCapabHooks(std::string server, std::vector<std::string> capabList) {
	
}

void Base::callServerBurstHooks(std::string server) {
	
}

void Base::callServerBurstEndHooks(std::string server) {
	
}

void Base::callOtherDataHooks(std::string server) {
	
}

void Base::callChanMsgOutHooks(std::string server, std::string client, std::string channel, char status, std::string message, Protocol* protoptr) {
	
}

void Base::callChanMsgSendHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	
}

void Base::callUserMsgOutHooks(std::string server, std::string client, std::string nick, std::string message, Protocol* protoptr) {
	
}

void Base::callUserMsgSendHooks(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::callChanNoticeOutHooks(std::string server, std::string client, std::string channel, char status, std::string message, Protocol* protoptr) {
	
}

void Base::callChanNoticeSendHooks(std::string server, std::string client, std::string channel, char status, std::string message) {
	
}

void Base::callUserNoticeOutHooks(std::string server, std::string client, std::string nick, std::string message, Protocol* protoptr) {
	
}

void Base::callUserNoticeSendHooks(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::callChanCTCPOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params, Protocol* protoptr) {
	
}

void Base::callChanCTCPSendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	
}

void Base::callUserCTCPOutHooks(std::string server, std::string client, std::string nick, std::string message, Protocol* protoptr) {
	
}

void Base::callUserCTCPSendHooks(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::callChanCTCPReplyOutHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params, Protocol* protoptr) {
	
}

void Base::callChanCTCPReplySendHooks(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	
}

void Base::callUserCTCPReplyOutHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params, Protocol* protoptr) {
	
}

void Base::callUserCTCPReplySendHooks(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	
}

Base::MutexManager::MutexManager(std::mutex* mutexPtr) : mutex(mutexPtr) {
	mutex->lock();
}

Base::MutexManager::~MutexManager() {
	mutex->unlock();
}