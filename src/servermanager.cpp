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
		throw ServerNotConfigured;
	auto protocolIter = serverConfig.find("protocol");
	if (protocolIter == serverConfig.end())
		throw ServerNoProtocol;
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
		newProto->pointManagers(modmanager, sockmanager);
		newProto->connectServer();
		serverServers.insert(std::pair<std::string, std::shared_ptr<ServerProtocol>> (protocolIter->second, newProto));
	} else {
		ClientProtocol*(*spawnCallFunc)(const std::string&) = static_cast<ClientProtocol*(*)(const std::string&)> (spawnFunc);
		std::shared_ptr<ClientProtocol> newProto (spawnCallFunc(server), std::bind(&ServerManager::unloadServer, this, server, protoFile, std::placeholders::_1));
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
	return clientServers.size() + serverServers.size();
}

void ServerManager::sendMsg(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendMsg, &ServerProtocol::sendMsg, server, source, target, message, tags);
}

void ServerManager::sendNotice(const std::string& server, const std::string& source, const std::string& target, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendNotice, &ServerProtocol::sendNotice, server, source, target, message, tags);
}

void ServerManager::setMode(const std::string& server, const std::string& source, const std::string& target, const std::list<std::tuple<bool, std::string, std::string>> modes, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setMode, &ServerProtocol::setMode, server, source, target, modes, tags);
}

void ServerManager::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::joinChan, server, client, channel, tags);
}

void ServerManager::joinChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& key, const std::map<std::string, std::string>& tags) {
	callClientHook(&ClientProtocol::joinChan, server, client, channel, key, tags);
}

void ServerManager::joinChanOver(const std::string& server, const std::string& client, const std::string& channel, const std::list<std::string>& statuses, time_t timestamp, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::joinChanOver, server, client, channel, statuses, timestamp, tags);
}

void ServerManager::partChan(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::partChan, &ServerProtocol::partChan, server, client, channel, reason, tags);
}

void ServerManager::kickUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::kickUser, &ServerProtocol::kickUser, server, source, channel, user, reason, tags);
}

void ServerManager::setTopic(const std::string& server, const std::string& source, const std::string& channel, const std::string& topic, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setTopic, &ServerProtocol::setTopic, server, source, channel, topic, tags);
}

void ServerManager::inviteUser(const std::string& server, const std::string& source, const std::string& channel, const std::string& user, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::inviteUser, &ServerProtocol::inviteUser, server, source, channel, user, tags);
}

void ServerManager::knock(const std::string& server, const std::string& client, const std::string& channel, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::knock, &ServerProtocol::knock, server, client, channel, reason, tags);
}

void ServerManager::changeNick(const std::string& server, const std::string& client, const std::string& nick, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::changeNick, &ServerProtocol::changeNic, server, client, nick, tags);
}

void ServerManager::sendPing(const std::string& server, const std::string& data, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendPing, &ServerProtocol::sendPing, server, data, tags);
}

void ServerManager::setAway(const std::string& server, const std::string& client, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setAway, &ServerProtocol::setAway, server, client, reason, tags);
}

void ServerManager::setUnaway(const std::string& server, const std::string& client, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::setUnaway, &ServerProtocol::setUnaway, server, client, tags);
}

void ServerManager::oper(const std::string& server, const std::string& client, const std::string& username, const std::string& password, const std::map<std::string, std::string>& tags) {
	callClientHook(&ClientProtocol::oper, server, client, username, password, tags);
}

void ServerManager::oper(const std::string& server, const std::string& client, const std::string& type, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::oper, server, client, type, tags);
}

void ServerManager::sendServerNotice(const std::string& server, const std::string& type, const std::string& message, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::sendServerNotice, server, type, message, tags);
}

void ServerManager::setMetadata(const std::string& server, const std::string& target, const std::string& key, const std::string& value, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::setMetadata, server, target, key, value, tags);
}

void ServerManager::setXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, time_t duration, const std::string& reason, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::setXLine, server, source, lineType, mask, duration, reason, tags);
}

void ServerManager::removeXLine(const std::string& server, const std::string& source, const std::string& lineType, const std::string& mask, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::removeXLine, server, source, lineType, mask, tags);
}

void ServerManager::changeIdent(const std::string& server, const std::string& user, const std::string& ident, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeIdent, server, user, ident, tags);
}

void ServerManager::changeHost(const std::string& server, const std::string& user, const std::string& host, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeHost, server, user, host, tags);
}

void ServerManager::changeGecos(const std::string& server, const std::string& user, const std::string& gecos, const std::map<std::string, std::string>& tags) {
	callServerHook(&ServerProtocol::changeGecos, server, user, gecos, tags);
}

void ServerManager::sendWallops(const std::string& server, const std::string& source, const std::string& message, const std::map<std::string, std::string>& tags) {
	callEitherHook(&ClientProtocol::sendWallops, &ServerProtocol::sendWallops, server, source, message, tags);
}

void ServerManager::pushLine(const std::string& server, const std::string& user, const IRCMessage* line) {
	callServerHook(&ServerProtocol::pushLine, server, user, line);
}

void ServerManager::sendOtherData(const std::string& server, const std::string& client, const IRCMessage* line) {
	callClientHook(&ClientProtocol::sendOtherData, server, client, line);
}

void ServerManager::sendOtherData(const std::string& server, const IRCMessage* line) {
	callServerHook(&ServerProtocol::sendOtherData, server, line);
}

std::string ServerManager::addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& host, const std::string& gecos) {
	return callServerHook(&ServerProtocol::addClient, server, nick, ident, host, gecos);
}

std::string ServerManager::addClient(const std::string& server, const std::string& nick, const std::string& ident, const std::string& gecos) {
	return callClientHook(&ClientProtocol::addClient, server, nick, ident, gecos);
}

void ServerManager::removeClient(const std::string& server, const std::string& client) {
	callEitherHook(&ClientProtocol::removeClient, &ServerProtocol::removeClient, server, client);
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
		throw ServerNotLoaded;
	return serverIter->second;
}

std::list<std::string> ServerManager::networkServerList(const std::string& server) {
	return callServerHook(&ServerProtocol::networkServerList, server);
}

std::list<std::string> ServerManager::xLineTypes(const std::string& server) {
	return callServerHook(&ServerProtocol::xLineTypes, server);
}

std::list<std::string> ServerManager::xLineList(const std::string& server, const std::string& lineType) {
	return callServerHook(&ServerProtocol::xLineList, server, lineType);
}

time_t ServerManager::xLineExpiry(const std::string& server, const std::string& lineType, const std::string& mask) {
	return callServerHook(&ServerProtocol::xLineExpiry, server, lineType, mask);
}

std::string ServerManager::xLineReason(const std::string& server, const std::string& lineType, const std::string& mask) {
	return callServerHook(&ServerProtocol::xLineReason, server, lineType, mask);
}

std::set<std::string> ServerManager::chanTypes(const std::string& server) {
	return callEitherHook(&ClientProtocol::chanTypes, &ServerProtocol::chanTypes, server);
}

std::list<std::pair<ModeType, std::string>> ServerManager::allChanModes(const std::string& server) {
	return callEitherHook(&ClientProtocol::allChanModes, &ServerProtocol::allChanModes, server);
}

char ServerManager::prefixSymbol(const std::string& server, const std::string& mode) {
	return callEitherHook(&ClientProtocol::prefixSymbol, &ServerProtocol::prefixSymbol, server, mode);
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, const std::string& status0, const std::string& status1) {
	return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, server, status0, status1);
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, const std::string& status0, char status1) {
	return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, status0, status1);
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, char status0, const std::string& status1) {
	return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, status1, status0);
}

std::pair<std::string, char> ServerManager::compareStatus(const std::string& server, char status0, char status1) {
	return callEitherHook(&ClientProtocol::compareStatus, &ServerProtocol::compareStatus, status0, status1);
}

std::list<std::string> ServerManager::chanList(const std::string& server) {
	return callServerHook(&ServerProtocol::chanList, server);
}

std::string ServerManager::chanTopic(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTopic, &ServerProtocol::chanTopic, server, channel);
}

std::string ServerManager::chanTopicSetter(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTopicSetter, &ServerProtocol::chanTopicSetter, server, channel);
}

time_t ServerManager::chanTimestamp(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanTimestamp, &ServerProtocol::chanTimestamp, server, channel);
}

std::list<std::string> ServerManager::chanUsers(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanUsers, &ServerProtocol::chanUsers, server, channel);
}

bool ServerManager::userInChan(const std::string& server, const std::string& channel, const std::string& user) {
	return callEitherHook(&ClientProtocol::userInChan, &ServerProtocol::userInChan, server, channel, user);
}

std::pair<std::string, char> ServerManager::userStatus(const std::string& server, const std::string& channel, const std::string& user) {
	return callEitherHook(&ClientProtocol::userStatus, &ServerProtocol::userStatus, server, channel, user);
}

bool ServerManager::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	return callEitherHook(&ClientProtocol::userHasStatus, &ServerProtocol::userHasStatus, server, channel, user, status);
}

bool ServerManager::userHasStatus(const std::string& server, const std::string& channel, const std::string& user, char status) {
	return callEitherHook(&ClientProtocol::userHasStatus, &ServerProtocol::userHasStatus, server, channel, user, status);
}

bool ServerManager::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, const std::string& status) {
	return callEitherHook(&ClientProtocol::userHasStatusOrGreater, &ServerProtocol::userHasStatusOrGreater, server, channel, user, status);
}

bool ServerManager::userHasStatusOrGreater(const std::string& server, const std::string& channel, const std::string& user, char status) {
	return callEitherHook(&ClientProtocol::userHasStatusOrGreater, &ServerProtocol::userHasStatusOrGreater, server, channel, user, status);
}

std::map<std::string, std::string> ServerManager::chanModes(const std::string& server, const std::string& channel) {
	return callEitherHook(&ClientProtocol::chanModes, &ServerProtocol::chanModes, server, channel);
}

bool ServerManager::chanHasMode(const std::string& server, const std::string& channel, const std::string& mode) {
	return callEitherHook(&ClientProtocol::chanHasMode, &ServerProtocol::chanHasMode, server, channel, mode);
}

std::string ServerManager::chanModeParam(const std::string& server, const std::string& channel, const std::string& mode) {
	return callEitherHook(&ClientProtocol::chanModeParam, &ServerProtocol::chanModeParam, server, channel, mode);
}

std::list<std::string> ServerManager::chanListModeList(const std::string& server, const std::string& channel, const std::string& listMode) {
	return callServerHook(&ServerProtocol::chanListModeList, server, channel, listMode);
}

std::list<std::string> ServerManager::clientList(const std::string& server) {
	return callEitherHook(&ClientProtocol::clientList, &ServerProtocol::clientList, server);
}

std::string ServerManager::userNick(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userNick, &ServerProtocol::userNick, server, user);
}

std::string ServerManager::userIdent(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userIdent, &ServerProtocol::userIdent, server, user);
}

std::string ServerManager::userHost(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userHost, &ServerProtocol::userHost, server, user);
}

std::string ServerManager::userGecos(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userGecos, &ServerProtocol::userGecos, server, user);
}

std::map<std::string, std::string> ServerManager::userModes(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userModes, &ServerProtocol::userModes, server, user);
}

bool ServerManager::userHasMode(const std::string& server, const std::string& user, const std::string& mode) {
	return callEitherHook(&ClientProtocol::userHasMode, &ServerProtocol::userHasMode, server, user, mode);
}

std::set<std::string> ServerManager::userChans(const std::string& server, const std::string& user) {
	return callEitherHook(&ClientProtocol::userChans, &ServerProtocol::userChans, server, user);
}

time_t ServerManager::userTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userTimestamp, server, user);
}

time_t ServerManager::userNickTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userNickTimestamp, server, user);
}

time_t ServerManager::userAwayTimestamp(const std::string& server, const std::string& user) {
	return callServerHook(&ServerProtocol::userAwayTimestamp, server, user);
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
			throw ServerNotLoaded;
		return ((*serverIter->second).*(serverFunc))(args...);
	}
	return ((*clientIter->second).*(clientFunc))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callClientHook(RetVal(ClientProtocol::*func)(Args&...), const std::string& server, Args&... args) {
	auto clientIter = clientServers.find(server);
	if (clientIter == clientServers.end()) {
		if (serverServers.find(server) == serverServers.end())
			throw ServerNotLoaded;
		throw ServerDoesNotSupport;
	}
	return ((*clientIter->second).*(func))(args...);
}

template<typename RetVal, typename... Args>
RetVal ServerManager::callServerHook(RetVal(ServerProtocol::*func)(Args&...), const std::string& server, Args&... args) {
	auto serverIter = serverServers.find(server);
	if (serverIter == serverServers.end()) {
		if (clientServers.find(server) == clientServers.end())
			throw ServerNotLoaded;
		throw ServerDoesNotSupport;
	}
	return ((*serverIter->second).*(func))(args...);
}