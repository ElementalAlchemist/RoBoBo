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



Base::MutexManager::MutexManager(std::mutex* mutexPtr) : mutex(mutexPtr) {
	mutex->lock();
}

Base::MutexManager::~MutexManager() {
	mutex->unlock();
}