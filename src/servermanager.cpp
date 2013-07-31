#include "servermanager.h"

void ServerManager::pointManagers(ModuleManager* mm, SocketManager* sm) {
	modmanager = mm;
	sockmanager = sm;
}

void ServerManager::connectStartupServers() {
	Config* config = Config::getHandle();
	std::list<std::unordered_map<std::string, std::string>> servers = config->getBlock("server");
	for (auto serverConf : servers) {
		auto nameIter = serverConf.find("name");
		auto startupIter = serverConf.find("startup");
		if (nameIter != serverConf.end() && startupIter != serverConf.end() && Config::makeBool(startupIter->second))
			connectServer(nameIter->second);
	}
}

void ServerManager::connectServer(const std::string& server) {
	Config* config = Config::getHandle();
	std::unordered_map<std::string, std::string> serverConfig = config->getSingleBlock("server", std::unordered_map<std::string, std::string> { { "name", server } });
	if (serverConfig.empty())
		throw ServerNotConfigured ();
	auto protocolIter = serverConfig.find("protocol");
	if (protocolIter == serverConfig.end())
		throw ServerNoProtocol ();
	void* protoFile = dlopen("protocols/" + protocolIter->second + ".so");
	if (protoFile == nullptr)
		throw ProtoLoadFailed (dlerror());
	void* serverFunc = dlsym(protoFile, "serverType");
	if (serverFunc == nullptr) {
		const char* loadError = dlerror();
		if (loadError)
			throw ProtoLoadFailed (loadError);
		throw ProtoLoadFailed ("The serverType symbol was set to null, but it must be a valid function.");
	}
	bool (*serverCallFunc)() = static_cast<bool(*)()>(serverFunc);
	bool isServer = serverCallFunc();
	void* spawnFunc = dlsym(protoFile, "spawn");
	if (spawnFunc == nullptr) {
		const char* loadError = dlerror();
		if (loadError)
			throw ProtoLoadFailed (loadError);
		throw ProtoLoadFailed ("The spawn symbol was set to null, but it must be a valid function.");
	}
	if (isServer) {
		ServerProtocol*(*spawnCallFunc)(const std::string&) = static_cast<ServerProtocol*(*)(const std::string&)> (spawnFunc);
		std::shared_ptr<ServerProtocol> newProto (spawnCallFunc(server), std::bind(&ServerManager::unloadServer, this, server, protoFile, std::placeholders::_1));
		if (protoAPIVersions.find(newProto->apiVersion()) == protoAPIVersions.end())
			throw ProtoAPIMismatch ();
		newProto->pointManagers(modmanager, sockmanager);
		newProto->connectServer();
		serverServers.insert(std::pair<std::string, std::shared_ptr<ServerProtocol>> (protocolIter->second, newProto));
	} else {
		ClientProtocol*(*spawnCallFunc)(const std::string&) = static_cast<ClientProtocol*(*)(const std::string&)> (spawnFunc);
		std::shared_ptr<ClientProtocol> newProto (spawnCallFunc(server), std::bind(&ServerManager::unloadServer, this, server, protoFile, std::placeholders::_1));
		if (protoAPIVersions.find(newProto->apiVersion()) == protoAPIVersions.end())
			throw ProtoAPIMismatch ();
		newProto->pointManagers(modmanager, sockmanager);
		newProto->connectServer();
		clientServers.insert(std::pair<std::string, std::shared_ptr<ClientProtocol>> (protocolIter->second, newProto));
	}
	serverTypes.insert(std::pair<std::string, std::string> (server, protocolIter->second));
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + server + " loaded and connected.");
}

void ServerManager::disconnectServer(const std::string& server) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		auto serverIter = serverServers.find(server);
		if (serverIter != serverServers.end()) {
			serverIter->second->disconnect();
			serverServers.erase(serverIter);
		}
	} else {
		clientIter->second->disconnect();
		clientServers.erase(clientIter);
	}
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + server + " disconnected.");
}

size_t ServerManager::checkServers() {
	std::list<std::unordered_map<std::string, std::shared_ptr<ClientProtocol>>::iterator> clientRemove;
	std::list<std::unordered_map<std::string, std::shared_ptr<ServerProtocol>>::iterator> serverRemove;
	for (auto clientIter = clientServers.begin(); clientIter != clientServers.end(); ++clientIter) {
		if (!clientIter->second->connected() && clientIter->second->shouldUnload())
			clientRemove.push_back(clientIter);
	}
	for (auto serverIter = serverServers.begin(); serverIter != serverServers.end(); ++serverIter) {
		if (!serverIter->second->connected() && serverIter->second->shouldUnload())
			serverRemove.push_back(serverIter);
	}
	for (auto remove : clientRemove)
		clientServers.erase(remove);
	for (auto remove : serverRemove)
		serverServers.erase(remove);
	return clientServers.size() + serverServers.size();
}

std::list<std::pair<std::string, bool>> ServerManager::activeServers(bool requestedByServerModule) {
	std::list<std::pair<std::string, bool>> serverList;
	for (auto client : clientServers)
		serverList.push_back(std::pair<std::string, bool> (client.first, !requestedByServerModule));
	for (auto server : serverServers)
		serverList.push_back(std::pair<std::string, bool> (client.first, requestedByServerModule));
	return serverList;
}

std::list<std::pair<std::string, bool>> ServerManager::connectedServers(bool requestedByServerModule) {
	std::list<std::pair<std::string, bool>> serverList;
	for (auto client : clientServers) {
		if (client.second->connected())
			serverList.push_back(std::pair<std::string, bool> (client.first, !requestedByServerModule));
	}
	for (auto server : serverServers) {
		if (server.second->connected())
			serverList.push_back(std::pair<std::string, bool> (server.first, requestedByServerModule));
	}
	return serverList;
}

std::string ServerManager::serverType(const std::string& server) {
	auto serverIter = serverTypes.find(server);
	if (serverIter == serverTypes.end())
		throw ServerNotLoaded ();
	return serverIter->second;
}

template<typename Protocol>
void ServerManager::unloadServer(const std::string& name, void* protoFile, Protocol* serverPtr) {
	delete serverPtr;
	dlclose(protoFile);
	serverTypes.erase(name);
	LogManager* logger = LogManager::getHandle();
	logger->log(LOG_DEBUG, "servers", "Server " + name + " unloaded.");
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callEitherHook(RetVal(ClientProtocol::*clientFunc)(Args&...), RetVal(ServerProtocol::*serverFunc)(Args&...), const std::string& server, Args&... args) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		auto serverIter = serverServers.find(server);
		if (serverIter == serverServers.end())
			throw ServerNotLoaded ();
		return ((*serverIter->second).*(serverFunc))(args...);
	}
	return ((*clientIter->second).*(clientFunc))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callClientHook(RetVal(ClientProtocol::*func)(Args&...), const std::string& server, Args&... args) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		if (serverServers.find(server) == serverServers.end())
			throw ServerNotLoaded ();
		throw ServerDoesNotSupport ();
	}
	return ((*clientIter->second).*(func))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callServerHook(RetVal(ServerProtocol::*func)(Args&...), const std::string& server, Args&... args) {
	auto serverIter = serverServers.find(server);
	if (serverIter == serverServers.end()) {
		if (clientServers.find(server) == clientServers.end())
			throw ServerNotLoaded ();
		throw ServerDoesNotSupport ();
	}
	return ((*serverIter->second).*(func))(args...);
}