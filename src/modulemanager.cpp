#include "modulemanager.h"

ModuleManager::ModuleManager(const std::string& wd) : workingDir(wd) {
	Config* config = Config::getHandle();
	config->addRehashNotify(std::bind(&ModuleManager::onRehash, this));
}

void ModuleManager::pointServerManager(ServerManager* sm) {
	servers = sm;
}

void ModuleManager::loadStartupModules() {
	// TODO: get modules from config
	// TODO: call openModule for all of them
	// TODO: handle provided services for all of them (just add service to the providers map)
	// TODO: call verifyModule for all of them
}

void ModuleManager::loadModule(const std::string& name) {
	// TODO: open each module
	// TODO: verify each module
}

void ModuleManager::unloadModule(const std::string& name) {
	// TODO: remove module's data from everything
	// TODO: remove empty provides, clients, dependents from maps
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
	// TODO: open module
	// TODO: get module instance
	// TODO: store module instance
}

void ModuleManager::verifyModule(std::shared_ptr<Module> mod) {
	// TODO: check provides
	// TODO: check requires, check with provided stuff
	// TODO: get client and server modules; store
	// TODO: call onLoadComplete
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