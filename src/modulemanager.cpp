#include "modulemanager.h"

ModuleManager::ModuleManager() {
	Config* config = Config::getHandle();
	config->addRehashNotify(std::bind(&ModuleManager::onRehash, this));
}

void ModuleManager::pointServerManager(ServerManager* sm) {
	servers = sm;
}

void ModuleManager::pointSocketManager(SocketManager* sm) {
	sockets = sm;
}

void ModuleManager::loadStartupModules() {
	/* Modules are loaded on startup by opening the files and instantiating the classes, pulling out what services they provide,
	 * and THEN verifying and storing the module, rather than simply by calling loadModule for each module we get from the
	 * configuration.  This is so that modules can be loaded on startup in any order without the order affecting whether modules
	 * will load and run properly.
	 */
	Config* conf = Config::getHandle();
	std::list<std::string> modulesToLoad = conf->getAllValues("module", "name");
	std::unordered_map<std::string, std::shared_ptr<Module>> openedModules;
	LogManager* logger = LogManager::getHandle();
	for (std::string modName : modulesToLoad) {
		try {
			std::shared_ptr<Module> mod = openModule(modName);
			const std::list<std::string> provided = mod->provides();
			for (std::string ability : provided) {
				if (providers.find(ability) == providers.end())
					providers.insert(std::pair<std::string, std::list<std::string>> (ability, std::list<std::string> ()));
			}
			openedModules.insert(std::pair<std::string, std::shared_ptr<Module>> (modName, mod));
		} catch (const ModuleAlreadyLoaded& ex) {
			logger->log(LOG_ERROR, "module-load", "The module " + modName + " was duplicated in the configuration.");
		} catch (const ModuleLoadFailed& ex) {
			logger->log(LOG_ERROR, "module-load", "The module " + modName + " could not be loaded: " + ex.what());
		} catch (const ModuleAPIMismatch& ex) {
			logger->log(LOG_ERROR, "module-load", "The module " + modName + " does not support the current module API.");
		} catch (const std::bad_alloc& ex) {
			logger->log(LOG_ERROR, "module-load", "Memory could not be allocated to load " + modName + ".");
			/* This log message is assuming that the issue is that the module is too big to fit in memory or something,
			 * or we may be in a restrained memory environment and the module that could conceivably fit was just not able to be allocated.
			 * In any case, I realize that getting the log handler and trying to write a log message MAY rethrow std::bad_alloc, but
			 * in that situation, we should probably let it go and shut everything down because we're basically too desperately out of
			 * memory for the core to run at that point.
			 */
		}
	}
	for (auto mod : openedModules) {
		verifyModule(mod.first, mod.second);
		logger->log(LOG_DEBUG, "module-load", "Module " + mod->first + " loaded successfully.");
	}
}

void ModuleManager::loadModule(const std::string& name) {
	verifyModule(name, openModule(name));
	/* This function, unlike the startup function above, should allow exceptions to fall through.
	 * This allows callers of the loadModule function to find out that something was wrong with
	 * loading and that they should respond to that somehow.
	 * This is not so allowable on startup as it must continue loading the startup modules and not let exceptions
	 * fall out of the main function.
	 */
}

void ModuleManager::unloadModule(const std::string& name) {
	auto modIter = loadedModules.find(name);
	if (modIter == loadedModules.end())
		throw ModuleNotLoaded;
	for (auto& action : registeredActions)
		action.second.remove(name);
	auto storedPriority = actionPriority.find(name);
	if (storedPriority != actionPriority.end())
		actionPriority.erase(storedPriority);
	std::list<std::string> emptyProviders, emptyClients, emptyDependents;
	for (auto& provider : providers) {
		provider.second.remove(name);
		if (provider.second.empty())
			emptyProviders.push_back(provider.first);
	}
	for (auto& client : clients) {
		client.second.remove(name);
		if (client.second.empty())
			emptyClients.push_back(client.first);
	}
	for (auto& dependent : dependents) {
		dependent.second.remove(name);
		if (dependent.second.empty())
			emptyDependents.push_back(dependent.first);
	}
	for (std::string provider : emptyProviders)
		providers.erase(provider);
	for (std::string client : emptyClients)
		clients.erase(client);
	for (std::string dependent : emptyDependents)
		dependents.erase(dependent);
	
	std::set<std::string> alsoUnload;
	for (auto dependent : dependents) {
		if (providers.find(dependent.first) == providers.end()) {
			for (std::string modName : dependent.second)
				alsoUnload.insert(modName);
		}
	}
	for (std::string modName : alsoUnload)
		unloadModule(modName);
	
	MutexLocker mutexLock (&queueMutex);
	actionQueue.push([=]() {
		auto modIter = loadedModules.find(name);
		auto modFileIter = moduleFiles.find(name);
		void* filePtr = modFileIter->second;
		modIter->second->onUnload();
		moduleFiles.erase(modFileIter);
		loadedModules.erase(modIter);
		clientModules.erase(name);
		serverModules.erase(name);
		for (auto mod : loadedModules)
			mod.second->onModuleUnload(name);
		dlclose(filePtr);
	});
}

bool ModuleManager::checkKeepAlive() const {
	for (auto module : loadedModules) {
		if (module.second->keepAlive())
			return true;
	}
	return false;
}

std::shared_ptr<Module> ModuleManager::modulePointer(const std::string& modName) const {
	auto modIter = loadedModules.find(modName);
	if (modIter == loadedModules.end())
		return std::shared_ptr<Module> (nullptr);
	return modIter->second;
}

void ModuleManager::addService(const std::string& modName, const std::string& service) {
	if (providers.find(service) != providers.end()) {
		const std::list<std::string>& providerList = providers[service];
		if (std::find(providerList.begin(), providerList.end(), modName) != providerList.end())
			return;
	}
	providers[service].push_back(modName);
}

void ModuleManager::removeService(const std::string& modName, const std::string& service) {
	auto serviceIter = providers.find(service);
	if (serviceIter == providers.end())
		return;
	auto modIter = std::find(serviceIter->second.begin(), serviceIter->second.end(), modName);
	if (modIter != serviceIter->second.end()) {
		serviceIter->second.erase(modIter);
		if (serviceIter->second.empty())
			providers.erase(serviceIter);
	}
}

void ModuleManager::addServiceClient(const std::string& modName, const std::string& service) {
	if (clients.find(service) != clients.end()) {
		const std::list<std::string>& clientList = clients[service];
		if (std::find(clientList.begin(), clientList.end(), modName) != clientList.end())
			return;
	}
	clients[service].push_back(modName);
}

void ModuleManager::removeServiceClient(const std::string& modName, const std::string& service) {
	auto serviceIter = clients.find(service);
	if (serviceIter == clients.end())
		return;
	auto modIter = std::find(serviceIter->second.begin(), serviceIter->second.end(), modName);
	if (modIter != serviceIter->second.end()) {
		serviceIter->second.erase(modIter);
		if (serviceIter->second.empty())
			clients.erase(serviceIter);
		removeServiceDependency(modName, service);
	}
}

void ModuleManager::addServiceDependency(const std::string& modName, const std::string& service) {
	if (providers.find(service) == providers.end())
		throw ServiceNotLoaded;
	if (dependents.find(service) != dependents.end()) {
		const std::list<std::string>& dependentList = dependents[service];
		if (std::find(dependentList.begin(), dependentList.end(), modName) != dependentList.end())
			return;
	}
	dependents[service].push_back(modName);
	addServiceClient(modName, service);
}

void ModuleManager::removeServiceDependency(const std::string& modName, const std::string& service) {
	auto serviceIter = dependents.find(service);
	if (serviceIter == dependents.end())
		return;
	auto modIter = std::find(serviceIter->second.begin(), serviceIter->second.end(), modName);
	if (modIter != serviceIter->second.end()) {
		serviceIter->second.erase(modIter);
		if (serviceIter->second.empty())
			dependents.erase(serviceIter);
	}
}

std::list<std::string> ModuleManager::providingModules(const std::string& capability) const {
	auto serviceIter = providers.find(capability);
	if (serviceIter == providers.end())
		return std::list<std::string> ();
	return serviceIter->second;
}

std::list<std::string> ModuleManager::usingModules(const std::string& capability) const {
	auto serviceIter = clients.find(capability);
	if (serviceIter == clients.end())
		return std::list<std::string> ();
	return serviceIter->second;
}

template<typename... Args>
void ModuleManager::callClientHook(ActionType type, Args... args) {
	if (type >= ServerHookLevel)
		throw HookTypeException;
	auto modIter = registeredActions.find(type);
	if (modIter == registeredActions.end())
		return;
	std::list<std::shared_ptr<ClientModule>> modList;
	for (std::string modName : modIter->second)
		modList.push_back(clientModules[modName]);
	MutexLocker mutexLock (&queueMutex);
	switch (type) {
		case HOOK_CLIENT_MESSAGE_CHANNEL:
			actionQueue.push(generateMsgHookCaller(&ClientModule::onChanMsg, modList, args...));
			break;
		case HOOK_CLIENT_MESSAGE_USER:
			actionQueue.push(generateMsgHookCaller(&ClientModule::onUserMsg, modList, args...));
			break;
		case HOOK_CLIENT_NOTICE_CHANNEL:
			actionQueue.push(generateMsgHookCaller(&ClientModule::onChanNotice, modList, args...));
			break;
		case HOOK_CLIENT_NOTICE_USER:
			actionQueue.push(generateMsgHookCaller(&ClientModule::onUserNotice, modList, args...));
			break;
		case HOOK_CLIENT_MODE_CHANNEL:
			actionQueue.push(generateHookCaller(&ClientModule::onChanMode, modList, args...));
			break;
		case HOOK_CLIENT_MODE_USER:
			actionQueue.push(generateHookCaller(&ClientModule::onUserMode, modList, args...));
			break;
		case HOOK_CLIENT_JOIN:
			actionQueue.push(generateHookCaller(&ClientModule::onJoin, modList, args...));
			break;
		case HOOK_CLIENT_PART:
			actionQueue.push(generateHookCaller(&ClientModule::onPart, modList, args...));
			break;
		case HOOK_CLIENT_QUIT:
			actionQueue.push(generateHookCaller(&ClientModule::onQuit, modList, args...));
			break;
		case HOOK_CLIENT_KICK:
			actionQueue.push(generateHookCaller(&ClientModule::onKick, modList, args...));
			break;
		case HOOK_CLIENT_TOPIC:
			actionQueue.push(generateHookCaller(&ClientModule::onTopic, modList, args...));
			break;
		case HOOK_CLIENT_INVITE:
			actionQueue.push(generateHookCaller(&ClientModule::onInvite, modList, args...));
			break;
		case HOOK_CLIENT_KNOCK:
			actionQueue.push(generateHookCaller(&ClientModule::onKnock, modList, args...));
			break;
		case HOOK_CLIENT_NICK:
			actionQueue.push(generateHookCaller(&ClientModule::onNick, modList, args...));
			break;
		case HOOK_CLIENT_PING:
			actionQueue.push(generateHookCaller(&ClientModule::onPing, modList, args...));
			break;
		case HOOK_CLIENT_PONG:
			actionQueue.push(generateHookCaller(&ClientModule::onPong, modList, args...));
			break;
		case HOOK_CLIENT_AWAY:
			actionQueue.push(generateHookCaller(&ClientModule::onAway, modList, args...));
			break;
		case HOOK_CLIENT_UNAWAY:
			actionQueue.push(generateHookCaller(&ClientModule::onUnaway, modList, args...));
			break;
		case HOOK_CLIENT_NUMERIC:
			actionQueue.push(generateHookCaller(&ClientModule::onNumeric, modList, args...));
			break;
		case HOOK_CLIENT_WALLOPS:
			actionQueue.push(generateHookCaller(&ClientModule::onWallops, modList, args...));
			break;
		case HOOK_CLIENT_CAP:
			actionQueue.push(generateHookCaller(&ClientModule::onCap, modList, args...));
			break;
		case HOOK_CLIENT_CONNECT_SELF:
			actionQueue.push(generateHookCaller(&ClientModule::onSelfConnect, modList, args...));
			break;
		case HOOK_CLIENT_QUIT_SELF:
			actionQueue.push(generateHookCaller(&ClientModule::onSelfQuit, modList, args...));
			break;
		case HOOK_CLIENT_OTHER:
			actionQueue.push(generateHookCaller(&ClientModule::onOtherData, modList, args...));
			break;
		case HOOK_CLIENT_OUT_MESSAGE_CHANNEL:
			actionQueue.push(generateChanOutHookCaller(&ClientModule::onOutChanMsg, modList, args...));
			break;
		case HOOK_CLIENT_OUT_MESSAGE_USER:
			actionQueue.push(generateUserOutHookCaller(&ClientModule::onOutUserMsg, modList, args...));
			break;
		case HOOK_CLIENT_OUT_NOTICE_CHANNEL:
			actionQueue.push(generateChanOutHookCaller(&ClientModule::onOutChanNotice, modList, args...));
			break;
		case HOOK_CLIENT_OUT_NOTICE_USER:
			actionQueue.push(generateUserOutHookCaller(&ClientModule::onOutUserNotice, modList, args...));
			break;
		case HOOK_CLIENT_SEND_MESSAGE_CHANNEL:
			actionQueue.push(generateHookCaller(&ClientModule::onSendChanMsg, modList, args...));
			break;
		case HOOK_CLIENT_SEND_MESSAGE_USER:
			actionQueue.push(generateHookCaller(&ClientModule::onSendUserMsg, modList, args...));
			break;
		case HOOK_CLIENT_SEND_NOTICE_CHANNEL:
			actionQueue.push(generateHookCaller(&ClientModule::onSendChanNotice, modList, args...));
			break;
		case HOOK_CLIENT_SEND_NOTICE_USER:
			actionQueue.push(generateHookCaller(&ClientModule::onSendUserNotice, modList, args...));
			break;
		default:
			throw HookTypeException;
	}
	startQueue();
}

template<typename... Args>
void ModuleManager::callServerHook(ActionType type, Args... args) {
	if (type < ServerHookLevel)
		throw HookTypeException;
	auto modIter = registeredActions.find(type);
	if (modIter == registeredActions.end())
		return;
	std::list<std::shared_ptr<ServerModule>> modList;
	for (std::string modName : modIter->second)
		modList.push_back(serverModules[modName]);
	MutexLocker mutexLock (&queueMutex);
	switch (type) {
		case HOOK_SERVER_MESSAGE_CHANNEL:
			actionQueue.push(generateMsgHookCaller(&ServerModule::onChanMsg, modList, args...));
			break;
		case HOOK_SERVER_MESSAGE_USER:
			actionQueue.push(generateMsgHookCaller(&ServerModule::onUserMsg, modList, args...));
			break;
		case HOOK_SERVER_NOTICE_CHANNEL:
			actionQueue.push(generateMsgHookCaller(&ServerModule::onChanNotice, modList, args...));
			break;
		case HOOK_SERVER_NOTICE_USER:
			actionQueue.push(generateMsgHookCaller(&ServerModule::onUserNotice, modList, args...));
			break;
		case HOOK_SERVER_MODE_CHANNEL:
			actionQueue.push(generateHookCaller(&ServerModule::onChanMode, modList, args...));
			break;
		case HOOK_SERVER_MODE_USER:
			actionQueue.push(generateHookCaller(&ServerModule::onUserMode, modList, args...));
			break;
		case HOOK_SERVER_JOIN:
			actionQueue.push(generateHookCaller(&ServerModule::onJoin, modList, args...));
			break;
		case HOOK_SERVER_PART:
			actionQueue.push(generateHookCaller(&ServerModule::onPart, modList, args...));
			break;
		case HOOK_SERVER_KICK:
			actionQueue.push(generateHookCaller(&ServerModule::onKick, modList, args...));
			break;
		case HOOK_SERVER_TOPIC:
			actionQueue.push(generateHookCaller(&ServerModule::onTopic, modList, args...));
			break;
		case HOOK_SERVER_INVITE:
			actionQueue.push(generateHookCaller(&ServerModule::onInvite, modList, args...));
			break;
		case HOOK_SERVER_KNOCK:
			actionQueue.push(generateHookCaller(&ServerModule::onKnock, modList, args...));
			break;
		case HOOK_SERVER_NICK:
			actionQueue.push(generateHookCaller(&ServerModule::onNick, modList, args...));
			break;
		case HOOK_SERVER_PING:
			actionQueue.push(generateHookCaller(&ServerModule::onPing, modList, args...));
			break;
		case HOOK_SERVER_PONG:
			actionQueue.push(generateHookCaller(&ServerModule::onPong, modList, args...));
			break;
		case HOOK_SERVER_AWAY:
			actionQueue.push(generateHookCaller(&ServerModule::onAway, modList, args...));
			break;
		case HOOK_SERVER_UNAWAY:
			actionQueue.push(generateHookCaller(&ServerModule::onUnaway, modList, args...));
			break;
		case HOOK_SERVER_CREATE_CHANNEL:
			actionQueue.push(generateHookCaller(&ServerModule::onChanCreate, modList, args...));
			break;
		case HOOK_SERVER_DESTROY_CHANNEL:
			actionQueue.push(generateHookCaller(&ServerModule::onChanDestroy, modList, args...));
			break;
		case HOOK_SERVER_REQUEST_STATS:
			actionQueue.push(generateHookCaller(&ServerModule::onRequestStats, modList, args...));
			break;
		case HOOK_SERVER_REQUEST_MOTD:
			actionQueue.push(generateHookCaller(&ServerModule::onRequestMOTD, modList, args...));
			break;
		case HOOK_SERVER_REQUEST_TIME:
			actionQueue.push(generateHookCaller(&ServerModule::onRequestTime, modList, args...));
			break;
		case HOOK_SERVER_REQUEST_ADMIN:
			actionQueue.push(generateHookCaller(&ServerModule::onRequestAdmin, modList, args...));
			break;
		case HOOK_SERVER_REQUEST_INFO:
			actionQueue.push(generateHookCaller(&ServerModule::onRequestInfo, modList, args...));
			break;
		case HOOK_SERVER_NOTICE_SERVER:
			actionQueue.push(generateHookCaller(&ServerModule::onServerNotice, modList, args...));
			break;
		case HOOK_SERVER_METADATA_CHANNEL:
			actionQueue.push(generateHookCaller(&ServerModule::onChanMetadata, modList, args...));
			break;
		case HOOK_SERVER_METADATA_USER:
			actionQueue.push(generateHookCaller(&ServerModule::onUserMetadata, modList, args...));
			break;
		case HOOK_SERVER_XLINE_ADD:
			actionQueue.push(generateHookCaller(&ServerModule::onAddXLine, modList, args...));
			break;
		case HOOK_SERVER_XLINE_REMOVE:
			actionQueue.push(generateHookCaller(&ServerModule::onRemoveXLine, modList, args...));
			break;
		case HOOK_SERVER_CONNECT_USER:
			actionQueue.push(generateHookCaller(&ServerModule::onUserConnect, modList, args...));
			break;
		case HOOK_SERVER_QUIT_USER:
			actionQueue.push(generateHookCaller(&ServerModule::onUserQuit, modList, args...));
			break;
		case HOOK_SERVER_CONNECT_USER_SELF:
			actionQueue.push(generateHookCaller(&ServerModule::onUserSelfConnect, modList, args...));
			break;
		case HOOK_SERVER_QUIT_USER_SELF:
			actionQueue.push(generateHookCaller(&ServerModule::onUserSelfQuit, modList, args...));
			break;
		case HOOK_SERVER_CHANGE_IDENT:
			actionQueue.push(generateHookCaller(&ServerModule::onChangeIdent, modList, args...));
			break;
		case HOOK_SERVER_CHANGE_HOST:
			actionQueue.push(generateHookCaller(&ServerModule::onChangeHost, modList, args...));
			break;
		case HOOK_SERVER_CHANGE_GECOS:
			actionQueue.push(generateHookCaller(&ServerModule::onChangeGecos, modList, args...));
			break;
		case HOOK_SERVER_WALLOPS:
			actionQueue.push(generateHookCaller(&ServerModule::onWallops, modList, args...));
			break;
		case HOOK_SERVER_CONNECT_SERVER:
			actionQueue.push(generateHookCaller(&ServerModule::onServerConnect, modList, args...));
			break;
		case HOOK_SERVER_QUIT_SERVER:
			actionQueue.push(generateHookCaller(&ServerModule::onServerQuit, modList, args...));
			break;
		case HOOK_SERVER_CAPAB:
			actionQueue.push(generateHookCaller(&ServerModule::onCapab, modList, args...));
			break;
		case HOOK_SERVER_BURST:
			actionQueue.push(generateHookCaller(&ServerModule::onBurst, modList, args...));
			break;
		case HOOK_SERVER_ENDBURST:
			actionQueue.push(generateHookCaller(&ServerModule::onEndBurst, modList, args...));
			break;
		case HOOK_SERVER_CONNECT_SELF:
			actionQueue.push(generateHookCaller(&ServerModule::onSelfConnect, modList, args...));
			break;
		case HOOK_SERVER_QUIT_SELF:
			actionQueue.push(generateHookCaller(&ServerModule::onSelfQuit, modList, args...));
			break;
		case HOOK_SERVER_OTHER:
			actionQueue.push(generateHookCaller(&ServerModule::onOtherData, modList, args...));
			break;
		case HOOK_SERVER_OUT_MESSAGE_CHANNEL:
			actionQueue.push(generateChanOutHookCaller(&ServerModule::onOutChanMsg, modList, args...));
			break;
		case HOOK_SERVER_OUT_MESSAGE_USER:
			actionQueue.push(generateUserOutHookCaller(&ServerModule::onOutUserMsg, modList, args...));
			break;
		case HOOK_SERVER_OUT_NOTICE_CHANNEL:
			actionQueue.push(generateChanOutHookCaller(&ServerModule::onOutChanNotice, modList, args...));
			break;
		case HOOK_SERVER_OUT_NOTICE_USER:
			actionQueue.push(generateUserOutHookCaller(&ServerModule::onOutUserNotice, modList, args...));
			break;
		default:
			throw HookTypeException;
	}
	startQueue();
}

void ModuleManager::onRehash() {
	for (auto module : loadedModules)
		module.second->onRehash();
}

std::shared_ptr<Socket> ModuleManager::assignSocket(const std::string& socketType) {
	// TODO: this when socket modules are implemented at all
	return std::shared_ptr<Socket> (nullptr);
}

std::list<std::string> ModuleManager::modules() {
	std::list<std::string> modList;
	for (auto module : loadedModules)
		modList.push_back(module.first);
	return modList;
}

std::list<std::string> ModuleManager::providedServices() {
	std::list<std::string> serviceList;
	for (auto service : providers)
		serviceList.push_back(service.first);
	return serviceList;
}

bool ModuleManager::serviceIsProvided(const std::string& service) {
	return providers.find(service) != providers.end();
}

std::list<std::string> ModuleManager::serviceProviders(const std::string& service) {
	auto serviceIter = providers.find(service);
	if (serviceIter == providers.end())
		return std::list<std::string> ();
	return serviceIter->second;
}

std::list<std::string> ModuleManager::serviceUsers(const std::string& service) {
	auto serviceIter = clients.find(service);
	if (serviceIter == clients.end())
		return std::list<std::string> ();
	return serviceIter->second;
}

std::shared_ptr<Module> ModuleManager::openModule(const std::string& name) {
	if (loadedModules.find(name) != loadedModules.end())
		throw ModuleAlreadyLoaded;
	void* modFile = dlopen(("modules/" + name + ".so").c_str(), RTLD_NOW);
	if (modFile == nullptr)
		throw ModuleLoadFailed (name, dlerror());
	void* spawnFunc = dlsym(modFile, "spawn");
	if (spawnFunc == nullptr) { // The spawn function should not be null in valid RoBoBo modules.
		const char* loadError = dlerror();
		dlclose(modFile);
		if (loadError)
			throw ModuleLoadFailed (name, loadError);
		throw ModuleLoadFailed (name, "The spawn symbol has been set to null, but it must be a valid function");
	}
	std::shared_ptr<Module>(*spawnCallFunc)(const std::string&) = static_cast<std::shared_ptr<Module>(*)(const std::string&)> (spawnFunc);
	std::shared_ptr<Module> newModule = spawnCallFunc(name);
	if (std::find(apiVersions.begin(), apiVersions.end(), newModule->apiVersion()) == apiVersions.end()) {
		dlclose(modFile);
		throw ModuleAPIMismatch (name);
	}
	moduleFiles.insert(std::pair<std::string, void*> (name, modFile));
	return newModule;
}

void ModuleManager::verifyModule(const std::string& name, std::shared_ptr<Module> mod) {
	std::list<std::string> requiredServices = mod->requires();
	for (std::string ability : requiredServices) {
		if (providers.find(ability) == providers.end()) {
			auto fileIter = moduleFiles.find(name);
			dlclose(fileIter->second);
			moduleFiles.erase(fileIter);
			throw ModuleRequirementsNotMet;
		}
	}
	loadedModules[name] = mod;
	std::list<std::string> providingServices = mod->provides();
	std::list<std::string> usingServices = mod->uses();
	for (std::string ability : providingServices)
		providers[ability].push_back(name);
	for (std::string ability : requiredServices) {
		dependents[ability].push_back(name);
		clients[ability].push_back(name);
	}
	for (std::string ability : usingServices)
		clients[ability].push_back(name);
	std::shared_ptr<ClientModule> clientMod = mod->clientModule();
	if (clientMod) {
		clientMod->loadManagerPointer(this, servers, sockets);
		clientModules[name] = clientMod;
	}
	std::shared_ptr<ServerModule> serverMod = mod->serverModule();
	if (serverMod) {
		serverMod->loadManagerPointer(this, servers, sockets);
		serverModules[name] = serverMod;
	}
	std::unordered_map<ActionType, std::unordered_map<Priority, std::set<std::string>, std::hash<int>>, std::hash<int>> actions = mod->registerActions();
	actionPriority[name] = actions;
	for (auto hook : actions) {
		std::set<std::string> before = hook.second[PRIORITY_BEFORE];
		std::set<std::string> after = hook.second[PRIORITY_AFTER];
		for (auto otherPriority : actionPriority) {
			if (otherPriority.second[hook.first][PRIORITY_BEFORE].find(mod->functionid()) != otherPriority.second[hook.first][PRIORITY_BEFORE].end())
				after.insert(loadedModules[otherPriority.first]->functionid());
			if (otherPriority.second[hook.first][PRIORITY_AFTER].find(mod->functionid()) != otherPriority.second[hook.first][PRIORITY_AFTER].end())
				before.insert(loadedModules[otherPriority.first]->functionid());
		}
		std::list<std::string>& actionList = registeredActions[hook.first];
		auto beforeIter = actionList.begin();
		auto afterIter = actionList.rbegin();
		for (; beforeIter != actionList.end() && before.find(loadedModules[*beforeIter]->functionid()) != before.end(); ++beforeIter) {}
		for (; afterIter != actionList.rend() && after.find(loadedModules[*afterIter]->function()) != after.end() && *beforeIter != *afterIter; ++afterIter) {}
		if (beforeIter == actionList.end() && afterIter == actionList.end()) {
			if (hook.second.find(PRIORITY_FIRST) != hook.second.end())
				actionList.push_front(name);
			else if (hook.second.find(PRIORITY_LAST) != hook.second.end())
				actionList.push_back(name);
			else {
				auto actIter = actionList.begin();
				size_t advanceAmt = actionList.size() / 2;
				for (size_t i = 0; i < advanceAmt; i++)
					actIter++;
				actionList.insert(actIter, name);
			}
		} else if (beforeIter == actionList.end())
			actionList.insert(afterIter.base(), name);
		else
			actionList.insert(beforeIter, name);
	}
	mod->onLoadComplete();
	for (auto mod : loadedModules) {
		if (mod.first != name)
			mod.second->onModuleLoad(name);
	}
}

template<typename ModType, typename... Args>
std::function<void()> ModuleManager::generateHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args) {
	return [=]() {
		for (std::string modName : modList) {
			try {
				((*modules.find(modName)->second).*(func))(args...);
			} catch (const std::exception& ex) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An exception has been thrown from a module. Module: " + modName + "; Details: " + ex.what());
			} catch (...) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An unknown exception has been thrown from a module (not derived from std::exception). Module: " + modName);
			}
		}
	};
}

template<typename ModType, typename... Args>
std::function<void()> ModuleManager::generateMsgHookCaller(MsgAction(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args) {
	return [=]() {
		for (std::string modName : modList) {
			try {
				MsgAction result = ((*modules.find(modName)->second).*(func))(args...);
				if (result == MSG_IGNORE)
					break;
			} catch (const std::exception& ex) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An exception has been thrown from a module. Module: " + modName + "; Details: " + ex.what());
			} catch (...) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An unknown exception has been thrown from a module (not derived from std::exception). Module: " + modName);
			}
		}
	};
}
	
template<typename ModType, typename... Args>
std::function<void()> ModuleManager::generateChanOutHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args) {
	std::string server, client, message;
	std::list<std::pair<std::string, char>> channels;
	std::map<std::string, std::string> tags;
	std::function<void(const std::string&, const std::list<std::pair<std::string, char>>&, const std::string&, const std::map<std::string, std::string>&)> callback;
	std::tie(server, client, channels, message, tags, callback) = std::tuple<Args...> (args...);
	return [=]() {
		for (std::string modName : modList) {
			try {
				((*modules.find(modName)->second).*(func))(server, client, channels, message, tags);
				if (channels.empty() || message.empty())
					return;
			} catch (const std::exception& ex) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An exception has been thrown from a module. Module: " + modName + "; Details: " + ex.what());
			} catch (...) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An unknown exception has been thrown from a module (not derived from std::exception). Module: " + modName);
			}
		}
		if (!channels.empty() && !message.empty())
			callback(client, channels, message, tags);
	};
}

std::function<void()> ModuleManager::generateUserOutHookCaller(void(ModType::*func)(Args...), const std::list<std::shared_ptr<ModType>>& modList, Args... args) {
	std::string server, client, message;
	std::list<std::string> users;
	std::map<std::string, std::string> tags;
	std::function<void(const std::string&, const std::list<std::string>&, const std::string&, const std::map<std::string, std::string>&)> callback;
	std::tie(server, client, users, message, tags, callback) = std::tuple<Args...> (args...);
	return [=]() {
		for (std::string modName : modList) {
			try {
				((*modules.find(modName).second).*(func))(server, client, users, message, tags);
				if (users.empty() || message.empty())
					return;
			} catch (std::exception& ex) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An exception has been thrown from a module. Module: " + modName + "; Details: " + ex.what());
			} catch (...) {
				LogManager* logger = LogManager::getHandle();
				logger->log(LOG_ERROR, "module-run", "An unknown exception has been thrown from a module (not derived from std::exception). Module: " + modName);
			}
		}
		if (!users.empty() && !message.empty())
			callback(client, users, message, tags);
	};
}

void ModuleManager::startQueue() {
	if (runningProcess)
		return;
	std::thread(&ModuleManager::processQueue, this);
	runningProcess = true;
}

void ModuleManager::processQueue() {
	if (actionQueue.empty()) {
		runningProcess = false;
		return;
	}
	while (true) {
		actionQueue.front()();
		MutexLocker mutexLock (&queueMutex);
		actionQueue.pop();
		if (actionQueue.empty())
			break;
	}
	runningProcess = false;
}