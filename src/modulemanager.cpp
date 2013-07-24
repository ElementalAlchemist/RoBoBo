#include "modulemanager.h"

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


template<ActionType, typename... Args> void ModuleManager::callHook(ActionType type, const Args&... args) {
	// TODO: this horrible function
}

void ModuleManager::rehash() {
	Config* conf = Config::getHandle();
	conf->readConfig();
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

void ModuleManager::processQueue() {
	runningProcess = true;
	// TODO: this
	runningProcess = false;
}