#include "base.h"

Base::Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log) : workingDir(working), configDir(config), configName(configFileName), debugLevel(debug), logDump(log), startup(true) {}

void Base::readConfiguration() {
	// TODO: parse config file
}

void Base::startQueueThread() {
	queueThread = std::thread(&Base::messageQueue);
}

void Base::loadModules() {
	for (std::string modName : startupModules)
		loadModule(modName);
	// Because this function is called at startup, also run through the requires/supports and onLoadCompletes
	std::list<std::string> unloadList; // Keep a list of modules to unload at the end to keep the loop sane
	for (std::pair<std::string, Module*> module : highModules) {
		std::list<std::string> modRequires = module.second->requires();
		for (std::string requirement : modRequires) {
			if (moduleServices[requirement].empty()) {
				std::cerr << "Module " << module.first << " requires the service " << requirement << ", which is not provided by another module." << std::endl;
				unloadList.push_back(module.first);
			}
			moduleSupports[requirement].push_back(module.first);
		}
		std::list<std::string> modSupports = module.second->supports();
		for (std::string supporting : modSupports)
			moduleSupports[supporting].push_back(module.first);
		if (!module.second->onLoadComplete())
			unloadList.push_back(module.first);
	}
	for (std::string modToUnload : unloadList)
		unloadModule(modToUnload);
}

void Base::connectServers() {
	for (std::string serverName : startupServers)
		connectServer(serverName);
}

void Base::checkServers() {
	// TODO: server check
}

void Base::unloadEverything() {
	for (std::pair<std::string, Protocol*> server : servers)
		disconnectServer(server.first);
	for (std::pair<std::string, Module*> module : highModules)
		unloadModule(module.first);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		unloadModule(module.first);
	for (std::pair<std::string, Module*> module : normalModules)
		unloadModule(module.first);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		unloadModule(module.first);
	for (std::pair<std::string, Module*> module : lowModules)
		unloadModule(module.first);
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
		return LOAD_ERROR;
	}
	// The spawn function of modules returns a module instance that we can use
	module_spawn_t moduleSpawn = dlsym(modFile, "spawn");
	Module* newModule = moduleSpawn(); // TODO: fill in with appropriate spawn parameters
	if (newModule->apiVersion() != 3000) {
		std::cerr << "Module " << modName << " is not compatible with this version of RoBoBo." << std::endl;
		return LOAD_INCOMPATIBLE;
	}
	// Add the module to the appropriate module list according to its priority
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
	if (!startup) {
		std::list<std::string> modRequires = newModule->requires();
		for (std::string requirement : modRequires) {
			// Check that the module's requirements are met
			if (moduleServices[requirement].empty()) {
				std::cerr << "Module " << modName << " requires the service " << requirement << ", which is not provided by another module." << std::endl;
				unloadModule(modName);
				return LOAD_FAILURE;
			}
			moduleSupports[requirement].push_back(modName);
		}
		std::list<std::string> modSupports = newModule->supports();
		for (std::string supporting : modSupports)
			moduleSupports[supporting].push_back(modName);
		if (!newModule->onLoadComplete()) {
			unloadModule(modName);
			return LOAD_FAILURE;
		}
	}
	return LOAD_SUCCESS;
}

void Base::unloadModule(std::string modName) {
	// TODO: also unload modules
}

void Base::connectServer(std::string server) {
	// TODO: basically everything
}

void Base::disconnectServer(std::string server) {
	// TODO: pretty much, what did you expect?
}

Socket* Base::loadSocket(std::string sockettype) {
	// TODO: load and return a socket
	return NULL;
}

void Base::messageQueue() {
	// TODO: process the queue once I get a better-defined list of types
}