#include "base.h"

Base::Base(std::string working, std::string config, std::string configFileName, unsigned short debug, bool log) : workingDir(working), configDir(config), configName(configFileName), debugLevel(debug), logDump(log), startup(true) {}

void Base::readConfiguration() {
	// TODO: parse config file
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
		if (module.second->onLoadComplete()) {
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
		} else
			unloadList.push_back(module.first);
	}
	for (std::string modToUnload : unloadList)
		unloadModule(modToUnload, false);
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
	// Every module is represented in moduleFiles, so just use that to unload all the modules
	for (std::pair<std::string, void*> module : moduleFiles)
		unloadModule(module.first, true);
}

void Base::modChanMsgHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	
}

void Base::modUserMsgHook(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::modChanNoticeHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
	
}

void Base::modUserNoticeHook(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::modChanCTCPHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	
}

void Base::modUserCTCPHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	
}

void Base::modChanCTCPReplyHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
	
}

void Base::modUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
	
}

void Base::modChanModeHook(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param) {
	
}

void Base::modUserModeHook(std::string server, std::string client, bool add, std::string mode) {
	
}

void Base::modUserSNOMaskHook(std::string server, std::string client, bool add, std::string snomask) {
	
}

void Base::modChanTopicHook(std::string server, std::string client, std::string channel, std::string topic) {
	
}

void Base::modChanJoinHook(std::string server, std::string client, std::string channel, std::string nick) {
	
}

void Base::modChanPartHook(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {
	
}

void Base::modUserConnectHook(std::string server, std::string nick) {
	
}

void Base::modUserQuitHook(std::string server, std::string client, std::string nick, std::string reason) {
	
}

void Base::modUserNickHook(std::string server, std::string client, std::string oldNick, std::string newNick) {
	
}

void Base::modNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {
	
}

void Base::modOperHook(std::string server, std::string nick, std::string operType) {
	
}

void Base::modSNoticeHook(std::string server, std::string snotype, std::string message) {
	
}

void Base::modUserMetadataHook(std::string server, std::string nick, std::string dataKey, std::string dataValue) {
	
}

void Base::modServerDataHook(std::string server, std::string dataType, std::string params) {
	
}

void Base::modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {
	
}

void Base::modXLineRemoveHook(std::string server, std::string lineType, std::string mask) {
	
}

void Base::modServerConnectHook(std::string server, std::string newServerName) {
	
}

void Base::modServerQuitHook(std::string server, std::string quitServerName, std::string reason) {
	
}

void Base::modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens) {
	
}

void Base::modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string &message) {
	
}

void Base::modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	
}

void Base::modUserMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string &message) {
	
}

void Base::modUserMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	
}

void Base::modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string &message) {
	
}

void Base::modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
	
}

void Base::modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string &message) {
	
}

void Base::modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message) {
	
}

void Base::modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {
	
}

void Base::modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	
}

void Base::modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {
	
}

void Base::modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	
}

void Base::modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {
	
}

void Base::modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
	
}

void Base::modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {
	
}

void Base::modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
	
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
	Module* newModule = moduleSpawn(modName, moduleConfig[modName], workingDir, debugLevel, this);
	if (newModule->apiVersion() != 3000) {
		std::cerr << "Module " << modName << " is not compatible with this version of RoBoBo." << std::endl;
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
				return LOAD_FAILURE;
			}
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
	for (std::pair<std::string, std::list<std::string>> service : moduleSupports)
		service.second.remove(modName);
	std::map<std::string, Module*>::iterator modEntry;
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

void Base::unloadSocket(std::string sockettype, Socket* socketptr) {
	// TODO: take socket and unregister it
}