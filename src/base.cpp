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
				return LOAD_NODEPENDS;
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

void Base::modChanMsgHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
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

void Base::modUserMsgHook(std::string server, std::string client, std::string nick, std::string message) {
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

void Base::modChanNoticeHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string message) {
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

void Base::modUserNoticeHook(std::string server, std::string client, std::string nick, std::string message) {
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

void Base::modChanCTCPHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
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

void Base::modUserCTCPHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
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

void Base::modChanCTCPReplyHook(std::string server, std::string client, std::string channel, char status, std::string nick, std::string ctcp, std::string data) {
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

void Base::modUserCTCPReplyHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string data) {
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

void Base::modChanModeHook(std::string server, std::string client, std::string channel, bool add, std::string mode, std::string param) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanMode(server, client, channel, add, mode, param);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanMode(server, client, channel, add, mode, param);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanMode(server, client, channel, add, mode, param);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanMode(server, client, channel, add, mode, param);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanMode(server, client, channel, add, mode, param);
	modHookMutex.unlock();
}

void Base::modUserModeHook(std::string server, std::string client, bool add, std::string mode) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserMode(server, client, add, mode);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserMode(server, client, add, mode);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserMode(server, client, add, mode);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserMode(server, client, add, mode);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserMode(server, client, add, mode);
	modHookMutex.unlock();
}

void Base::modUserSNOMaskHook(std::string server, std::string client, bool add, std::string snomask) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserSNOMask(server, client, add, snomask);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserSNOMask(server, client, add, snomask);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserSNOMask(server, client, add, snomask);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserSNOMask(server, client, add, snomask);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserSNOMask(server, client, add, snomask);
	modHookMutex.unlock();
}

void Base::modChanTopicHook(std::string server, std::string client, std::string channel, std::string topic) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanTopic(server, client, channel, topic);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanTopic(server, client, channel, topic);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanTopic(server, client, channel, topic);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanTopic(server, client, channel, topic);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanTopic(server, client, channel, topic);
	modHookMutex.unlock();
}

void Base::modChanJoinHook(std::string server, std::string client, std::string channel, std::string nick) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanJoin(server, client, channel, nick);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanJoin(server, client, channel, nick);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanJoin(server, client, channel, nick);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanJoin(server, client, channel, nick);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanJoin(server, client, channel, nick);
	modHookMutex.unlock();
}

void Base::modChanPartHook(std::string server, std::string client, std::string channel, std::string nick, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onChanPart(server, client, channel, nick, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onChanPart(server, client, channel, nick, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onChanPart(server, client, channel, nick, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onChanPart(server, client, channel, nick, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onChanPart(server, client, channel, nick, reason);
	modHookMutex.unlock();
}

void Base::modUserConnectHook(std::string server, std::string nick) {
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

void Base::modUserQuitHook(std::string server, std::string client, std::string nick, std::string reason) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserQuit(server, client, nick, reason);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserQuit(server, client, nick, reason);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserQuit(server, client, nick, reason);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserQuit(server, client, nick, reason);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserQuit(server, client, nick, reason);
	modHookMutex.unlock();
}

void Base::modUserNickHook(std::string server, std::string client, std::string oldNick, std::string newNick) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserNick(server, client, oldNick, newNick);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserNick(server, client, oldNick, newNick);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserNick(server, client, oldNick, newNick);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserNick(server, client, oldNick, newNick);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserNick(server, client, oldNick, newNick);
	modHookMutex.unlock();
}

void Base::modNumericHook(std::string server, std::string client, std::string numeric, std::vector<std::string> data) {
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

void Base::modOperHook(std::string server, std::string nick, std::string operType) {
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

void Base::modSNoticeHook(std::string server, std::string snotype, std::string message) {
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

void Base::modUserMetadataHook(std::string server, std::string nick, std::string dataKey, std::string dataValue) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onUserMetadata(server, nick, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onUserMetadata(server, nick, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onUserMetadata(server, nick, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onUserMetadata(server, nick, dataKey, dataValue);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onUserMetadata(server, nick, dataKey, dataValue);
	modHookMutex.unlock();
}

void Base::modServerDataHook(std::string server, std::string dataType, std::vector<std::string> params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules)
		module.second->onServerData(server, dataType, params);
	for (std::pair<std::string, Module*> module : mediumHighModules)
		module.second->onServerData(server, dataType, params);
	for (std::pair<std::string, Module*> module : normalModules)
		module.second->onServerData(server, dataType, params);
	for (std::pair<std::string, Module*> module : mediumLowModules)
		module.second->onServerData(server, dataType, params);
	for (std::pair<std::string, Module*> module : lowModules)
		module.second->onServerData(server, dataType, params);
	modHookMutex.unlock();
}

void Base::modXLineAddHook(std::string server, std::string lineType, std::string mask, std::string setter, time_t expiry, std::string reason) {
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

void Base::modXLineRemoveHook(std::string server, std::string lineType, std::string mask) {
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

void Base::modServerConnectHook(std::string server, std::string newServerName) {
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

void Base::modServerQuitHook(std::string server, std::string quitServerName, std::string reason) {
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

void Base::modOtherDataHook(std::string server, std::string client, std::vector<std::string> lineTokens) {
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

void Base::modChanMsgOutHook(std::string server, std::string client, std::string channel, char status, std::string &message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanMsgOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	modHookMutex.unlock();
}

void Base::modChanMsgSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
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

void Base::modUserMsgOutHook(std::string server, std::string client, std::string nick, std::string &message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserMsgOut(server, client, nick, message);
		if (message == "")
			return;
	}
	modHookMutex.unlock();
}

void Base::modUserMsgSendHook(std::string server, std::string client, std::string nick, std::string message) {
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

void Base::modChanNoticeOutHook(std::string server, std::string client, std::string channel, char status, std::string &message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanNoticeOut(server, client, channel, status, message);
		if (message == "")
			return;
	}
	modHookMutex.unlock();
}

void Base::modChanNoticeSendHook(std::string server, std::string client, std::string channel, char status, std::string message) {
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

void Base::modUserNoticeOutHook(std::string server, std::string client, std::string nick, std::string &message) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserNoticeOut(server, client, nick, message);
		if (message == "")
			return;
	}
	modHookMutex.unlock();
}

void Base::modUserNoticeSendHook(std::string server, std::string client, std::string nick, std::string message) {
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

void Base::modChanCTCPOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanCTCPOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	modHookMutex.unlock();
}

void Base::modChanCTCPSendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
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

void Base::modUserCTCPOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserCTCPOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	modHookMutex.unlock();

}

void Base::modUserCTCPSendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
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

void Base::modChanCTCPReplyOutHook(std::string server, std::string client, std::string channel, char status, std::string &ctcp, std::string &params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onChanCTCPReplyOut(server, client, channel, status, ctcp, params);
		if (ctcp == "")
			return;
	}
	modHookMutex.unlock();

}

void Base::modChanCTCPReplySendHook(std::string server, std::string client, std::string channel, char status, std::string ctcp, std::string params) {
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

void Base::modUserCTCPReplyOutHook(std::string server, std::string client, std::string nick, std::string &ctcp, std::string &params) {
	modHookMutex.lock();
	for (std::pair<std::string, Module*> module : highModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumHighModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : normalModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : mediumLowModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	for (std::pair<std::string, Module*> module : lowModules) {
		module.second->onUserCTCPReplyOut(server, client, nick, ctcp, params);
		if (ctcp == "")
			return;
	}
	modHookMutex.unlock();

}

void Base::modUserCTCPReplySendHook(std::string server, std::string client, std::string nick, std::string ctcp, std::string params) {
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